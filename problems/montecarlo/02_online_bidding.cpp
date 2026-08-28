// =============================================================================
//  [モンテカルロ系 02] オンライン広告入札 (競合が確率的)
// =============================================================================
//  【問題】
//    T 回のオークションが順番に行われる。t 回目の枠の価値 v_t は事前にすべて分かっている。
//    毎回、自分の入札額 b (0..BMAX の 5 刻み、b <= 残予算) を決める。
//    競合の最高入札額 c_t は 0..CMAX の一様乱数で、入札した後にしか分からない。
//    b > c_t なら落札し、利益 v_t - b を得て、予算が b 減る。b <= c_t なら何も起きない (予算も減らない)。
//    総利益を最大化せよ。
//  【入力】
//    T BMAX CMAX BUDGET
//    v_0 ... v_{T-1}
//    c_0 ... c_{T-1}    (ジャッジ側。解答側は turn に到達するまで見てはいけない)
//  【出力】
//    T 行。そのターンの入札額。
//  【スコア】 総利益 (大きいほど良い)。予算超過があれば 0 点。
//  【入力生成方法】
//    T=150, BMAX=100, CMAX=80, BUDGET=1500 固定。
//    v_t は 20..150 の一様整数。c_t は 0..CMAX の一様整数。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     入札した瞬間には落札できるか分からず、しかも予算が有限。
//     「今 高く入札して取りに行く」か「安く済ませて後の良い枠に予算を残す」かのトレードオフになる。
//     各入札額について未来の競合入札 c を 1 本引いて最後までシミュレートし、平均利益が最大の額を選ぶ。
//     ★全候補が同じ c の列を使うので比較は完全に公平。
//     この問題は rollout の方策を**理論的に厳密に**作れるので、そこを詰めると素直に伸びる。
//
//   ● 状態 (State)
//     残り予算、ターン、直前に出した入札額 (結果は次のターンに判明する)。
//
//   ● 手の作り方
//     0, 5, 10, ..., BMAX のうち残り予算以下の額。
//
//   ● 評価値 / rollout の方策
//     c が [0, CMAX] の一様分布なので、入札 b の勝率は min(b, R) / R (R = CMAX + 1)。
//     期待利得 Σ (b/R)(v - b) を 期待支出 Σ b^2 / R <= 予算 の下で最大化すると、
//     ラグランジュ乗数 λ を使って
//       b = v / (2(1 + λ)),  (1 + λ) = max(1, sqrt(Σ残り v^2 / (4 R 予算)))
//     と閉じた形で書ける。
//     さらに予算は 5 の倍数しか動かないので、(ターン, 残り予算 / 5) の 2 次元 DP (150 × 301) で
//     **厳密な最適方策そのもの**を前計算できる (POLICY=1)。
//     DP が張れない大きさの入力では上の λ 近似に落ちる。
//
//   ● つまずきポイント
//     ・初版の方策は「b = 0.6 × v」で、予算 1500 に対して使いすぎだった。
//       rollout が「予算の価値」を過小評価するので、本番の判断も甘くなる。
//     ・1 手のブレが落札の有無に直結するのでスコアの揺れが大きい。
//       A/B はシナリオ数を固定した決定的な条件でも取ると判断しやすい。
//
//   ● さらに伸ばすなら
//     ・競合の分布が未知という設定にして、観測から推定する形にする
//     ・候補の絞り込み (CAND_W=4) は -0.5% で不採用だった。候補が少ないので絞る旨みが薄い
//
//  【改善】
//    rollout の方策を「b = 0.6 x v (予算を使い切る勢いで入札)」から
//    「残り予算と残り枠から決まる最適入札 (予算 DP。作れなければラグランジュ近似)」に置き換えた。
//    元の方策は予算 1500 に対して使いすぎで、rollout が「予算の価値」を過小評価していた。
//    seed 0..4 合計: 変更前 15817 -> 変更後 16166 (+2.2%)
//    1 手のブレが落札の有無に直結してスコアの揺れが大きいので、シナリオ数を固定した決定的な条件でも確認:
//      seed 0..9   変更前 29910 -> 変更後 30673 (+2.6%)
//      seed 10..19 変更前 32686 -> 変更後 33300 (+1.9%)   ※調整に使っていない seed
//
//  【採用したライブラリ】 monte carlo/1_monte_carlo
//    実測比較 (seed 0,1,2 の合計スコア): 1_monte_carlo=9171 / 2_successive_halving=8970
// =============================================================================
// =============================================================================
//  モンテカルロ法 (Monte Carlo)   ---  AHC 用 高速テンプレート
// =============================================================================
//  各候補手について同じシナリオ集合を試し、期待値が最良の手を選ぶ。全候補が必ず同数のシナリオを試す完全公平版。
//
//  【この手法が向く問題】
//    未来が確率的に決まる問題 (何が来るか分からない / 相手の行動が分からない 等)。
//    「今この手を選んだら、最終的に平均でどれくらいのスコアになるか」を
//    ランダムな未来(シナリオ)を何本も試して見積もり、期待値が最良の手を選ぶ。
//
//  【★シナリオを公平にするための 4 原則★】  ← ここが最重要
//    1. 1 ラウンドで作ったシナリオは「全候補」で使い回す (共通乱数法)。
//       候補ごとに別のシナリオを引くと、運の良いシナリオを引いた候補が勝ってしまう。
//       同じ未来で比べると差だけが残るので、少ないシナリオ数でも正しく比較できる。
//    2. シナリオは「ターン番号で引ける配列」にする。乱数を消費順に引くと、
//       候補によって行動が変わった瞬間に未来そのものが変わってしまう。
//    3. rollout の中で方策がランダムに動く場合、その乱数も候補間で同じ種から始める。
//    4. 時間切れの打ち切りは必ず「ラウンド境界」で行う。ラウンドの途中で切ると
//       候補ごとにシナリオ数が変わり、比較が壊れる。
//    ※ 全候補のシナリオ数が必ず等しくなるので、平均を取らず「合計」のまま比較できる(除算不要)。
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct Move     { ... };                  ... 手
//    struct State    { float score; ... };     ... 状態 (score は必須)
//    struct Scenario { ... };                  ... 未来の不確定要素 1 本ぶん
//    init_state(State&)                        ... 初期状態
//    enum_moves(const State&, Move*)           ... 今の候補手を全列挙して個数を返す
//    calc_score(const State&, Move)            ... その手で確定するスコア差分
//    apply_move(State&, Move)                  ... 手を適用する (score は触らない)
//    reveal_turn(State&, int t)                ... ターン t で初めて分かる情報を state に入れる
//    gen_scenario(Scenario&, Xor128&, int)     ... シナリオを 1 本作る
//    rollout(State&, const Scenario&, Xor128&) ... 最後までシミュレートして最終スコアを返す
//
//  【高速化のポイント】
//    ・シナリオは 1 ラウンドに 1 本だけ作り、全候補で共有する (生成コストを候補数で割れる)
//    ・候補を適用した直後の状態を 1 回だけ作り置きし、rollout ごとにそこからコピーする
//    ・シナリオ数は時間から自動決定。1 ラウンドの実測時間で「次が入るか」を判定して打ち切る
//    ・期待値の集計は double (float だと数万回の加算で精度が落ちる)
//    ・時間計測は rdtsc/cntvct 直読み、1 ラウンドに 1 回だけ
// =============================================================================
#ifdef __x86_64__
#pragma GCC target("avx2")
#endif
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// =============================================================================
// 1. 高速乱数 (xorshift128)
// =============================================================================
struct Xor128 {
    uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    inline void seed(uint32_t s) {
        x = 123456789u ^ s; y = 362436069u; z = 521288629u; w = 88675123u ^ (s * 2654435761u);
        for (int i = 0; i < 16; i++) next();
    }
    inline uint32_t next() {
        uint32_t t = x ^ (x << 11);
        x = y; y = z; z = w;
        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    }
    inline uint32_t next(uint32_t n) { return (uint32_t)(((uint64_t)next() * n) >> 32); }   // [0,n)
    inline uint32_t next(uint32_t l, uint32_t r) { return l + next(r - l); }
    inline float nextf() { return (float)(next() >> 8) * (1.0f / 16777216.0f); }
};
static Xor128 rng;

// =============================================================================
// 2. 高速タイマー (rdtsc / cntvct を直接読む)
// =============================================================================
struct Timer {
    static inline uint64_t tick() {
#if defined(__x86_64__) || defined(_M_X64)
        uint32_t lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t)hi << 32) | lo;
#elif defined(__aarch64__)
        uint64_t t;
        __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(t));
        return t;
#else
        return (uint64_t)chrono::steady_clock::now().time_since_epoch().count();
#endif
    }
    uint64_t t0_ = 0;
    double   ms_per_tick_ = 1e-6;
    int      calib_ = 0;
    chrono::steady_clock::time_point w0_;

    void start() {
#if defined(__aarch64__)
        uint64_t f; __asm__ __volatile__("mrs %0, cntfrq_el0" : "=r"(f));
        ms_per_tick_ = 1000.0 / (double)f; calib_ = 100;
#elif defined(__x86_64__) || defined(_M_X64)
        ms_per_tick_ = 1.0 / 2.8e6; calib_ = 0;
#else
        ms_per_tick_ = 1e-6; calib_ = 100;
#endif
        w0_ = chrono::steady_clock::now();
        t0_ = tick();
    }
    inline float ms() {
        uint64_t d = tick() - t0_;
        if (calib_ < 4) calibrate(d);
        return (float)((double)d * ms_per_tick_);
    }
    void calibrate(uint64_t d) {
        double w = chrono::duration<double, milli>(chrono::steady_clock::now() - w0_).count();
        if (w > 0.25 * (double)(1u << (2 * calib_))) { ms_per_tick_ = w / (double)d; calib_++; }
    }
};
static Timer timer;

// =============================================================================
// 3. パラメータ (optuna から環境変数 p1, p2, ... で上書きできる)
// =============================================================================
template <class T> inline void pick_env(const char *key, T &dst) {
    if (const char *s = getenv(key)) {
        char *e = nullptr;
        double v = strtod(s, &e);
        if (e != s) dst = (T)v;
    }
}

// ---- 調整パラメータ (p1, p2, ... が optimize.py のキーと対応) ----
float ROLLOUT_TH = 1.0f;   // p1: rollout の入札方策の強さ (1 = 理論値そのまま)
float TIME_ALPHA = 0.0f;   // p2: ターンごとの時間配分 (0=均等, 大きいほど序盤に厚く)
int   POLICY     = 1;      // p3: rollout の方策 (0 = ラグランジュ近似, 1 = 予算 DP の厳密解)
int   CAND_W     = 0;      // p4: 候補を方策の推奨値の周り +-CAND_W*5 に絞る (0 = 絞らない)
int   MAX_SCENARIO = 1 << 30;  // 1 ターンで回すシナリオ数の上限 (MAX_SCENARIO=1 で貪欲相当)

void load_params() {
    pick_env("p1", ROLLOUT_TH);
    pick_env("p2", TIME_ALPHA);
    pick_env("p3", POLICY);
    pick_env("p4", CAND_W);
    pick_env("MAX_SCENARIO", MAX_SCENARIO);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化
// constexpr bool MAXIMIZE = false;    // ← スコア最小化

inline bool is_better(double a, double b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }
constexpr double WORST_SCORE = MAXIMIZE ? -1e300 : 1e300;

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS = 1900.0f;   // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr float ROUND_MARGIN  = 1.10f;     // 「次のラウンドが入るか」の判定に掛ける安全係数
// (この手法は探索の設定より「rollout をどれだけ軽くできるか」が効くので、ここは少なめ)

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_rounds = 0, g_rollouts = 0;
static int g_min_sc = INT32_MAX, g_max_sc = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (オンライン広告入札)
// #############################################################################
constexpr int MAXT = 400;
constexpr int MAX_CAND = 32;

int T, BMAX, CMAX, BUDGET;
static int VV[MAXT], TRUE_C[MAXT];

struct Move { int16_t b; };

struct State {
    float   score;
    int32_t budget;
    int32_t turn;
    int16_t lastb;
};

static void build_policy();

inline void reveal_turn(State &s, int turn) {
    if (turn > 0) {
        const int c = TRUE_C[turn - 1];
        if (s.lastb > c) { s.score += (float)(VV[turn - 1] - s.lastb); s.budget -= s.lastb; }
    }
    s.turn = turn;
}

void init_state(State &s) { build_policy(); s.score = 0.0f; s.budget = BUDGET; s.turn = 0; s.lastb = 0; }

// ---- 方策のための前計算 ----
//   R  = 競合入札の取りうる値の個数 (勝率 = min(b,R)/R)
//   S2[t] = 残り枠の v^2 の総和。ラグランジュ乗数の計算に使う。
//   BPOL[t][k] = 予算 5k のときにターン t で入れるべき入札額 (予算 DP の厳密解)
constexpr int MAXK = 1024;
static int   R_ = 1;
static double S2[MAXT + 1];
static int8_t BPOL[MAXT][MAXK];
static int    NK = 0;
static bool   USE_DP = false;

// ラグランジュ近似: 予算の影の価格 lam を残り枠から解析的に求め、b = v / (2(1+lam)) を入れる。
//   期待利得 sum (b/R)(v-b) を 期待支出 sum b^2/R <= 予算 の下で最大化した解。
static inline int policy_bid(int t, int bud) {
    if (bud <= 0) return 0;
    if (USE_DP) {
        int k = bud / 5; if (k >= NK) k = NK - 1;
        return (int)BPOL[t][k] * 5;
    }
    double lam = 1.0;
    const double q = S2[t] / (4.0 * (double)R_ * (double)bud);
    if (q > 1.0) lam = sqrt(q);
    int b = (int)((double)ROLLOUT_TH * (double)VV[t] / (2.0 * lam));
    const int cap = min(min(BMAX, bud), R_ - 1 > 0 ? R_ - 1 : 0);
    if (b > cap) b = cap;
    if (b < 0) b = 0;
    return (b / 5) * 5;
}

static void build_policy() {
    R_ = CMAX + 1;
    S2[T] = 0.0;
    for (int t = T - 1; t >= 0; t--) S2[t] = S2[t + 1] + (double)VV[t] * (double)VV[t];
    NK = BUDGET / 5 + 1;
    USE_DP = (POLICY != 0 && NK <= MAXK && T <= MAXT && BMAX / 5 <= 127);
    if (!USE_DP) return;
    // 予算 DP: f[t][k] = 残り予算 5k でターン t 以降に得られる期待利得の最大値
    static double f0[MAXK], f1[MAXK];
    for (int k = 0; k < NK; k++) f1[k] = 0.0;
    for (int t = T - 1; t >= 0; t--) {
        for (int k = 0; k < NK; k++) {
            double best = -1e18; int bb = 0;
            for (int b = 0; b <= BMAX; b += 5) {
                if (b > k * 5) break;
                const double pw = (double)min(b, R_) / (double)R_;
                const double val = pw * ((double)VV[t] - (double)b + f1[k - b / 5]) + (1.0 - pw) * f1[k];
                if (val > best) { best = val; bb = b; }
            }
            f0[k] = best; BPOL[t][k] = (int8_t)(bb / 5);
        }
        for (int k = 0; k < NK; k++) f1[k] = f0[k];
    }
}

inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    const int hi = min(BMAX, s.budget);
    if (CAND_W > 0) {                       // 方策の推奨値の周りだけを候補にする
        const int c = policy_bid(s.turn, s.budget);
        int lo = c - CAND_W * 5, up = c + CAND_W * 5;
        if (lo < 0) lo = 0;
        if (up > hi) up = hi;
        for (int b = lo; b <= up && m < MAX_CAND; b += 5) out[m++].b = (int16_t)b;
        if (m > 0) return m;
    }
    for (int b = 0; b <= hi && m < MAX_CAND; b += 5) out[m++].b = (int16_t)b;
    if (m == 0) out[m++].b = 0;
    return m;
}
inline float calc_score(const State &s, const Move &mv) { (void)s; (void)mv; return 0.0f; }  // 結果は次ターンに判明
inline void apply_move(State &s, const Move &mv) { s.lastb = mv.b; }

struct Scenario { int16_t c[MAXT]; };
inline void gen_scenario(Scenario &sc, Xor128 &rnd, int from) {
    for (int t = max(0, from - 1); t < T; t++) sc.c[t] = (int16_t)rnd.next((uint32_t)(CMAX + 1));
}

// ★必須: s の状態から最後までシミュレートして最終利益を返す
//   方策は「残り予算と残り枠から決まる最適入札」。予算の価値を正しく織り込むのが肝。
inline float rollout(State &s, const Scenario &sc, Xor128 &rnd) {
    (void)rnd;
    float total = s.score;
    int bud = s.budget;
    // まず直前に決めた入札を解決する
    if (s.turn < T) {
        const int c = sc.c[s.turn];
        if (s.lastb > c && s.lastb <= bud) { total += (float)(VV[s.turn] - s.lastb); bud -= s.lastb; }
    }
    for (int t = s.turn + 1; t < T; t++) {
        const int b = policy_bid(t, bud);
        if (b > sc.c[t]) { total += (float)(VV[t] - b); bud -= b; }
    }
    return total;
}

void read_input() {
    if (scanf("%d %d %d %d", &T, &BMAX, &CMAX, &BUDGET) == 4 && T >= 1) {
        T = min(T, MAXT);
        for (int t = 0; t < T; t++) scanf("%d", &VV[t]);
        for (int t = 0; t < T; t++) scanf("%d", &TRUE_C[t]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 402);
        T = 150; BMAX = 100; CMAX = 80; BUDGET = 1500;
        for (int t = 0; t < T; t++) { VV[t] = 20 + (int)g.next(131); TRUE_C[t] = (int)g.next((uint32_t)(CMAX + 1)); }
    }
}

void output(const vector<Move> &hist) {
    string r; r.reserve(hist.size() * 4);
    for (const Move &mv : hist) { r += to_string((int)mv.b); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &hist) {
    float total = 0.0f; int bud = BUDGET;
    for (int t = 0; t < T && t < (int)hist.size(); t++) {
        const int b = hist[t].b;
        if (b < 0 || b > BMAX) { fprintf(stderr, "[error] 入札額が範囲外\n"); return -1.0f; }
        if (b > TRUE_C[t]) {
            if (b > bud) { fprintf(stderr, "[error] 予算超過\n"); return -1.0f; }
            total += (float)(VV[t] - b); bud -= b;
        }
    }
    return total;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
//  ターンごとの時間配分
// =============================================================================
//  TIME_ALPHA = 0 なら全ターン均等。大きくすると序盤に多く時間を割り当てる。
//  各ターンの締切は「累積」で持つので、早く終わったターンの余りは自動的に後ろへ回る。
static float g_deadline[MAXT + 1];
static void plan_time(float begin_ms, float total_ms) {
    double sum = 0.0;
    for (int t = 0; t < T; t++) sum += pow((double)(T - t) / (double)T, (double)TIME_ALPHA);
    double acc = 0.0;
    for (int t = 0; t < T; t++) {
        acc += pow((double)(T - t) / (double)T, (double)TIME_ALPHA);
        g_deadline[t] = begin_ms + total_ms * (float)(acc / sum);
    }
}

// ---- 1 ターンぶんの作業領域 (静的確保。探索中に malloc しない) ----
static Move     g_cand[MAX_CAND];      // 候補手
static State    g_base[MAX_CAND];      // 候補を適用した直後の状態 (作り置き)
static double   g_sum [MAX_CAND];      // 候補ごとのスコア合計 (float だと精度が落ちる)
static State    g_work;                // rollout 用の作業状態
static Scenario g_sc;                  // ★シナリオは 1 本だけ持ち、全候補で共有する
static uint32_t g_round_id = 0;        // シナリオの種 (ターンをまたいで別の未来を引く)

// =============================================================================
// 7. モンテカルロ法 本体
// =============================================================================
//  1 ラウンド = 「シナリオを 1 本作り、全候補でそれを試す」。
//  ラウンド単位で回すので、全候補のシナリオ数が常に等しくなる (＝完全に公平)。
Move choose_move(const State &state, float deadline_ms) {
    const int C = enum_moves(state, g_cand);
    if (C <= 1) { g_min_sc = 0; return g_cand[0]; }      // 選択の余地なし → シミュレート不要

    // 候補を適用した直後の状態を作り置き (rollout ごとに作り直さない)
    for (int c = 0; c < C; c++) {
        g_base[c] = state;
        g_base[c].score += calc_score(state, g_cand[c]);
        apply_move(g_base[c], g_cand[c]);
        g_sum[c] = 0.0;
    }

    int   n     = 0;                    // 回したシナリオ数
    float prev  = timer.ms();
    float round = 0.0f;                 // 直近 1 ラウンドの実測時間
    while (n < MAX_SCENARIO) {
        // ★打ち切りはラウンド境界だけ。途中で切ると候補ごとにシナリオ数が変わって不公平になる
        if (n > 0 && prev + round * ROUND_MARGIN > deadline_ms) break;

        // --- シナリオを 1 本作る (全候補で共有 = 共通乱数法) ---
        Xor128 rs; rs.seed(g_round_id++);
        gen_scenario(g_sc, rs, state.turn + 1);
        Xor128 rp; rp.seed(0x9E3779B9u ^ g_round_id);    // 方策用の乱数 (候補間で同じ種)

        // --- 全候補を同じシナリオで試す ---
        for (int c = 0; c < C; c++) {
            g_work = g_base[c];
            Xor128 r = rp;                               // ★候補ごとに同じ状態から始める
            g_sum[c] += (double)rollout(g_work, g_sc, r);
        }
        n++;
        const float now = timer.ms();
        round = now - prev;
        prev  = now;
    }
    g_rounds += n; g_rollouts += (ll)n * C;
    g_min_sc = min(g_min_sc, n); g_max_sc = max(g_max_sc, n);

    // 全候補のシナリオ数が同じなので、平均を取らず合計のまま比較できる
    int best = 0;
    for (int c = 1; c < C; c++) if (is_better(g_sum[c], g_sum[best])) best = c;
    return g_cand[best];
}

// =============================================================================
// 8. main
// =============================================================================
int main() {
    timer.start();      // ★ 一番最初にタイマー開始
    load_params();      // 環境変数からパラメータを読む (optuna 用)
    read_input();

    static State state;
    init_state(state);
    plan_time(timer.ms(), TIME_LIMIT_MS - timer.ms());   // ターンごとの締切を決める

    vector<Move> hist(T);
    for (int t = 0; t < T; t++) {
        reveal_turn(state, t);              // ★このターンに初めて分かる情報を state に取り込む
        const Move mv = choose_move(state, g_deadline[t]);
        hist[t] = mv;
        state.score += calc_score(state, mv);
        apply_move(state, mv);
    }
    // ★最終ターンの実現値を state に反映する。これが無いと下の [check] の running 側が
    //   最後の 1 ターンぶんだけ足りない値になり、差分計算のバグと区別できなくなる。
    if (T < MAXT) reveal_turn(state, T);
    output(hist);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)replay_true_score(hist));
    fprintf(stderr, "rounds = %lld, rollouts = %lld, scenarios/turn = %d..%d, time = %.1f ms\n",
            g_rounds, g_rollouts, g_min_sc == INT32_MAX ? 0 : g_min_sc, g_max_sc, (double)timer.ms());
    // ★実装のバグ検出: 進行中に積み上げたスコアと、出力を再生した真のスコアを比べる
    //   ※ 最終ターンの実現値は「次の reveal_turn()」で反映される作りなので、running 側は
    //      最後の 1 ターンぶんだけ足りない値になる。これは仕様であってバグではない。
    //      提出スコアは replay 側 (上の Score = ...) を見ること。それ以上にずれていたら
    //      calc_score / apply_move / reveal_turn のどれかが間違っている
    fprintf(stderr, "[check] running = %.0f / replay = %.0f\n",
            (double)state.score, (double)replay_true_score(hist));
    return 0;
}
