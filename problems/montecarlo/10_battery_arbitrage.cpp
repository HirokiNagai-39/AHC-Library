// =============================================================================
//  [モンテカルロ系 10] 蓄電池の売買計画 (価格が確率的)
// =============================================================================
//  【問題】
//    容量 CAP の蓄電池を持っている (初期残量 0)。T ターンにわたって電力を売買する。
//    毎ターンの朝に「その日の価格 p_t」が分かる。その上で次のいずれかを選ぶ。
//      ・充電: 残量を 1 増やし p_t を支払う (残量 < CAP のときのみ)
//      ・放電: 残量を 1 減らし p_t を受け取る (残量 > 0 のときのみ)
//      ・何もしない
//    翌日以降の価格は分からない。価格はランダムウォークで
//    p_{t+1} = clamp(p_t + (-D..D の一様整数), PMIN, PMAX) に従うことだけ分かっている。
//    T ターンの総利益 (受取 - 支払) を最大化せよ。
//  【入力】
//    T CAP PMIN PMAX D
//    p_0 ... p_{T-1}   (ジャッジ側の価格。turn に到達するまで見てはいけない)
//  【出力】
//    T 行。そのターンの行動 (1=充電, -1=放電, 0=何もしない)。
//  【スコア】 総利益 (大きいほど良い)。容量・残量の制約を破ったら 0 点。
//  【入力生成方法】
//    T=200, CAP=20, PMIN=10, PMAX=90, D=8 固定。p_0 = 50。
//    p_{t+1} = clamp(p_t + (-D..D の一様整数), PMIN, PMAX)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「今の価格は安いが、もっと安くなるかもしれない」という典型的な確率的タイミング問題。
//     各行動 (充電 / 放電 / 何もしない) について未来の価格をランダムウォークで 1 本引いて
//     最後までシミュレートし、平均利益が最大の行動を選ぶ。
//     ★全候補が同じ価格の列を使うので、たまたま値上がりした候補が勝つことがない (共通乱数法)。
//
//   ● 状態 (State)
//     電池の残量、今日の価格、ターン。
//
//   ● 手の作り方
//     充電 (残量 < CAP のとき) / 放電 (残量 > 0 のとき) / 何もしない。
//
//   ● 評価値 / rollout の方策
//     **在庫連動のしきい値方策**。
//       基準価格 mid = (PMIN + PMAX)/2 + ROLLOUT_TH × (1 - 2 × 残量率)
//       mid - BAND より安ければ充電、mid + BAND より高ければ放電。
//     残量 0 なら基準が高くなり (多少高くても買う)、満量なら基準が低くなる (多少安くても売る)。
//     残り日数 n が少ないときは持てる量を min(CAP, n-1) に縮め、e >= n なら無条件に放電する。
//     この形は「価格を状態に入れた厳密 DP (200 × 81 × 21)」の最適方策をなぞったもので、
//     DP の最適しきい値が**残量に対してほぼ線形**(残量 0 で 82、満量で 18、v(e)+v(21-e)=100 と完全対称)
//     だったことから導いた。これで DP 最適値の 95% 前後まで到達する。
//
//   ● つまずきポイント
//     ・**gen_scenario がランダムウォークを「今日の価格」ではなく初日の定数 50 から始めていた。**
//       ただしこれ単独では ±0% だった (元の方策が価格水準を活かせていなかったため)。
//       モデルの誤りは、方策がその情報を使えるようになって初めてスコアに効く。
//     ・スコアの絶対値が小さく seed による差が大きい (73〜775)。A/B は 20〜30 seed で取ること。
//       実際、パラメータの微調整は seed 0-29 と 30-59 でランキングが逆転する完全なノイズだったので、
//       理論値の近傍に固定してある。
//
//   ● さらに伸ばすなら
//     ・厳密 DP の方策をそのまま rollout に載せる (前計算のメモリと時間との相談)
//     ・価格過程を AR(1) など自己相関のあるモデルにして、推定込みの問題にする方向
//
//  【改善】
//    (1) gen_scenario の起点を「初日の価格 50」から「今日の価格」に直した (ランダムウォークの起点)。
//    (2) rollout の方策を単純なしきい値から在庫連動のしきい値に変更した。
//        残量 0 なら基準を高く (多少高くても買う)、満量なら基準を低く (多少安くても売る)。
//        残り日数 n が少ないときは持てる量を min(CAP, n-1) に縮め、e >= n なら無条件に放電する。
//        この形は価格を状態に入れた厳密 DP の最適方策 (残量に対しほぼ線形なしきい値) をなぞったもの。
//    seed 0..4 合計: 変更前 2080 -> 変更後 2454 (+18.0%)   ※ seed 0..29 では 10161 -> 13721 (+35.0%)
//
//  【採用したライブラリ】 monte carlo/1_monte_carlo
//    実測比較 (seed 0,1,2 の合計スコア): 1_monte_carlo=1267 / 2_successive_halving=1267
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
float ROLLOUT_TH = 25.0f;  // p1: rollout の基準価格の傾き (残量 0 -> +TH, 満量 -> -TH)
float TIME_ALPHA = 0.0f;   // p2: ターンごとの時間配分 (0=均等, 大きいほど序盤に厚く)
float BAND       = 2.0f;   // p3: 基準価格からこの幅だけ離れたら売買する (不感帯)
int   MAX_SCENARIO = 1 << 30;  // 1 ターンで回すシナリオ数の上限 (MAX_SCENARIO=1 で貪欲相当)

void load_params() {
    pick_env("p1", ROLLOUT_TH);
    pick_env("p2", TIME_ALPHA);
    pick_env("p3", BAND);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (蓄電池の売買計画)
// #############################################################################
constexpr int MAXT = 400;
constexpr int MAX_CAND = 3;

int T, CAP, PMIN, PMAX, DSTEP;
static int TRUE_P[MAXT];

struct Move { int8_t a; };            // 1=充電, -1=放電, 0=何もしない

struct State {
    float   score;
    int32_t e;                        // 残量
    int32_t turn, price;
};

static int g_price_now = 50;          // 今日の価格 (reveal 済みの情報。gen_scenario の起点に使う)

inline void reveal_turn(State &s, int turn) { s.price = TRUE_P[turn]; s.turn = turn; g_price_now = s.price; }

void init_state(State &s) { s.score = 0.0f; s.e = 0; s.turn = 0; s.price = 0; }

inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    if (s.e < CAP) out[m++].a = 1;
    if (s.e > 0)   out[m++].a = -1;
    out[m++].a = 0;
    return m;
}
inline float calc_score(const State &s, const Move &mv) { return -(float)mv.a * (float)s.price; }
inline void apply_move(State &s, const Move &mv) { s.e += mv.a; }

struct Scenario { int16_t p[MAXT]; };
inline void gen_scenario(Scenario &sc, Xor128 &rnd, int from) {
    // 現在価格からランダムウォークで未来を作る (from-1 は今日なので実際の値を使う)
    int p = g_price_now;
    for (int t = max(0, from); t < T; t++) {
        p = p + (int)rnd.next((uint32_t)(2 * DSTEP + 1)) - DSTEP;
        p = min(PMAX, max(PMIN, p));
        sc.p[t] = (int16_t)p;
    }
}

// rollout の方策: 「残量が多いほど安く売りたくなる」在庫連動のしきい値。
//   基準価格 mid = 中央値 + ROLLOUT_TH * (1 - 2 * 残量率)
//   ・残量 0      -> mid が高い  = 多少高くても買う
//   ・残量が満杯  -> mid が低い  = 多少安くても売る
//   さらに残り日数 n で「実際に持てる上限」を min(CAP, n-1) に縮め、
//   売り切れない量 (e >= n) は無条件に放電する (期末に残しても 0 点なので)。
inline float rollout(State &s, const Scenario &sc, Xor128 &rnd) {
    (void)rnd;
    float total = s.score;
    int e = s.e;
    const float mid0 = (float)(PMIN + PMAX) * 0.5f;
    for (int t = s.turn + 1; t < T; t++) {
        const int   n = T - t;                       // 今日を含む残り日数
        const float p = (float)sc.p[t];
        if (e >= n) { e--; total += p; continue; }   // 残り日数では売り切れない -> 必ず放電
        const int   capn = min(CAP, n - 1);          // 期末までに捌ける量が上限
        const float frac = (capn > 0) ? (float)e / (float)capn : 1.0f;
        const float mid  = mid0 + ROLLOUT_TH * (1.0f - 2.0f * frac);
        if (p <= mid - BAND) { if (e < CAP) { e++; total -= p; } }
        else if (p >= mid + BAND) { if (e > 0) { e--; total += p; } }
    }
    return total;
}

void read_input() {
    if (scanf("%d %d %d %d %d", &T, &CAP, &PMIN, &PMAX, &DSTEP) == 5 && T >= 1) {
        T = min(T, MAXT);
        for (int t = 0; t < T; t++) scanf("%d", &TRUE_P[t]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 410);
        T = 200; CAP = 20; PMIN = 10; PMAX = 90; DSTEP = 8;
        int p = 50;
        for (int t = 0; t < T; t++) {
            TRUE_P[t] = p;
            p = p + (int)g.next((uint32_t)(2 * DSTEP + 1)) - DSTEP;
            p = min(PMAX, max(PMIN, p));
        }
    }
}

void output(const vector<Move> &hist) {
    string r; r.reserve(hist.size() * 3);
    for (const Move &mv : hist) { r += to_string((int)mv.a); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &hist) {
    float total = 0.0f; int e = 0;
    for (int t = 0; t < T && t < (int)hist.size(); t++) {
        const int a = hist[t].a;
        if (a > 1 || a < -1) { fprintf(stderr, "[error] 不正な行動\n"); return -1.0f; }
        if (a == 1 && e >= CAP) { fprintf(stderr, "[error] 容量超過\n"); return -1.0f; }
        if (a == -1 && e <= 0) { fprintf(stderr, "[error] 残量不足\n"); return -1.0f; }
        e += a;
        total -= (float)a * (float)TRUE_P[t];
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
