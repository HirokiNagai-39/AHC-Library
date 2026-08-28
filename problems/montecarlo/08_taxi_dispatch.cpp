// =============================================================================
//  [モンテカルロ系 08] タクシー配車 (乗車要求が確率的に発生)
// =============================================================================
//  【問題】
//    1 次元の道路 (座標 0..L) に K 台のタクシーがある。T ターンのあいだ運行する。
//    毎ターン、確率 QREQ で乗車要求 (乗車地 a, 目的地 b) が発生する (発生と a,b はそのターンに判明)。
//    要求が発生したら、空いているタクシーを 1 台選んで割り当てるか、断るかを決める。
//    タクシー k (位置 pos_k) を割り当てると、|pos_k - a| + |a - b| ターンのあいだ塞がり、
//    利益 |a-b| * FARE - |pos_k - a| * MCOST を得て、終了後の位置は b になる。
//    T ターンの総利益を最大化せよ。
//  【入力】
//    T K L FARE MCOST QREQ(×10000)
//    has_t a_t b_t   (T 行。ジャッジ側の要求。turn に到達するまで見てはいけない)
//  【出力】
//    T 行。割り当てたタクシー番号 (断る/要求無しなら -1)。
//  【スコア】 総利益 (大きいほど良い)。塞がっているタクシーを割り当てたら 0 点。
//  【入力生成方法】
//    T=200, K=5, L=100, FARE=3, MCOST=2, QREQ=0.6 固定。初期位置は 0..L の一様整数。
//    各ターン、確率 QREQ で要求が発生し、a,b は 0..L の一様整数 (a != b)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     今この要求を受けると、そのタクシーはしばらく塞がり、その間に来るかもしれない
//     「もっとおいしい要求」を取り逃す。未来の要求が確率的なので、
//     各候補 (どのタクシーを割り当てるか / 断るか) について未来の要求列を 1 本引いて
//     最後までシミュレートし、平均利益が最大の手を選ぶ。★全候補が同じ要求列を使うので比較が公平。
//     この問題の本質は**時間が足りないこと**。依頼は T×QREQ = 120 件来るのに、
//     5 台では 25 件ほどしか捌けない。つまり「利益の絶対額」ではなく
//     「タクシーの時間 1 ターンあたりいくら稼げるか」で選ぶのが正しい。
//
//   ● 状態 (State)
//     各タクシーの「あと何ターン塞がっているか」と現在位置、今回の要求 (乗車地・目的地)、ターン。
//
//   ● 手の作り方
//     空いているタクシーに割り当てる K 通り + 断る 1 通り。
//
//   ● 評価値 / rollout の方策
//     「利益 > 機会費用 × 塞がるターン数」のときだけ、最も近い空車を割り当てる。
//     ROLLOUT_TH = 2.2 が「1 ターンあたりいくら稼げれば受ける価値があるか」の値。
//     終盤は塞がっても損しないので、機会費用を残りターン数で頭打ちにする。
//
//   ● つまずきポイント
//     ・**gen_scenario が本物の生成と違っていた**。実際の入力は a != b (乗車地と目的地が異なる) だが、
//       シナリオでは a == b を許していたため、運賃 0 の要求が混ざってモデルがずれていた。
//       シナリオ生成は必ず本物の分布と一致させること。
//     ・「利益がしきい値以上なら受ける」という絶対額での判断だと、
//       遠い客を運ぶ長時間の仕事を受けてしまい、その間の稼ぎを丸ごと失う。
//
//   ● さらに伸ばすなら
//     ・空車数に応じて機会費用を変える案、目的地が端に寄る依頼を嫌う案、
//       中央付近の空車を温存する案はいずれも悪化した
//     ・伸ばすなら「タクシーごとの機会費用」を近似 DP で求めて割当に使う方向
//
//  【改善】
//    (1) rollout の受注判断を「利益 > しきい値」から「利益 > 機会費用 × 塞がるターン数」に変更した。
//        依頼は T×QREQ=120 件来るのに 5 台では 25 件ほどしか捌けないので、
//        時間あたりの採算 (ROLLOUT_TH=2.2/ターン) が合う依頼だけを受けるのが正しい。
//        終盤は塞がっても損しないので機会費用を残りターン数で頭打ちにする。
//    (2) gen_scenario の乗車地・目的地を a != b にして本物の入力と同じ分布にした。
//    seed 0..4 合計: 変更前 10805 -> 変更後 12017 (+11.2%)   ※ seed 0..9 でも全 seed 改善 (+11.3%)
//
//  【採用したライブラリ】 monte carlo/1_monte_carlo
//    実測比較 (seed 0,1,2 の合計スコア): 1_monte_carlo=6758 / 2_successive_halving=6748
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
float ROLLOUT_TH = 2.2f;   // p1: rollout で受注を判断する「1 ターンあたりの機会費用」
float TIME_ALPHA = 0.0f;   // p2: ターンごとの時間配分 (0=均等, 大きいほど序盤に厚く)
int   MAX_SCENARIO = 1 << 30;  // 1 ターンで回すシナリオ数の上限 (MAX_SCENARIO=1 で貪欲相当)

void load_params() {
    pick_env("p1", ROLLOUT_TH);
    pick_env("p2", TIME_ALPHA);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (タクシー配車)
// #############################################################################
constexpr int MAXT = 400, MAXK = 8;
constexpr int MAX_CAND = MAXK + 1;

int T, K, L, FARE, MCOST, QREQ;
static int TRUE_HAS[MAXT], TRUE_A[MAXT], TRUE_B[MAXT], INIT_POS[MAXK];

struct Move { int8_t car; };      // -1 なら断る

struct State {
    float   score;
    int32_t busy[MAXK], pos[MAXK];
    int32_t turn, has, ra, rb;
};

inline void reveal_turn(State &s, int turn) {
    if (turn > 0) for (int k = 0; k < K; k++) if (s.busy[k] > 0) s.busy[k]--;
    s.has = TRUE_HAS[turn]; s.ra = TRUE_A[turn]; s.rb = TRUE_B[turn];
    s.turn = turn;
}

void init_state(State &s) {
    s.score = 0.0f; s.turn = 0; s.has = 0; s.ra = 0; s.rb = 0;
    for (int k = 0; k < K; k++) { s.busy[k] = 0; s.pos[k] = INIT_POS[k]; }
}

inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    if (s.has) for (int k = 0; k < K; k++) if (s.busy[k] == 0) out[m++].car = (int8_t)k;
    out[m++].car = -1;
    return m;
}
inline float calc_score(const State &s, const Move &mv) {
    if (mv.car < 0 || !s.has) return 0.0f;
    return (float)abs(s.ra - s.rb) * (float)FARE - (float)abs(s.pos[mv.car] - s.ra) * (float)MCOST;
}
inline void apply_move(State &s, const Move &mv) {
    if (mv.car < 0 || !s.has) return;
    s.busy[mv.car] = abs(s.pos[mv.car] - s.ra) + abs(s.ra - s.rb);
    s.pos[mv.car] = s.rb;
}

struct Scenario { int8_t has[MAXT]; int16_t a[MAXT], b[MAXT]; };
inline void gen_scenario(Scenario &sc, Xor128 &rnd, int from) {
    for (int t = max(0, from); t < T; t++) {
        sc.has[t] = (int8_t)((int)rnd.next(10000) < QREQ);
        const int a = (int)rnd.next((uint32_t)(L + 1));
        int b = (int)rnd.next((uint32_t)L);            // b != a (本物の入力と同じ分布にする)
        if (b >= a) b++;
        sc.a[t] = (int16_t)a; sc.b[t] = (int16_t)b;
    }
}

// rollout の方策: 「利益」ではなく「1 ターンあたりの利益」で受けるかを決める。
//   タクシーは K 台 * T ターンぶんの時間しか無く、1 件受けると (移動 + 乗車) ターン塞がる。
//   その間に来る他の依頼を捨てることになるので、塞がる時間 1 ターンあたり ROLLOUT_TH の
//   機会費用を払うとみなし、利益がそれを上回るときだけ受ける。
//   ・終盤は塞がっても失うものが無いので、機会費用は残りターン数までで頭打ちにする。
inline float rollout(State &s, const Scenario &sc, Xor128 &rnd) {
    (void)rnd;
    float total = s.score;
    int busy[MAXK], pos[MAXK];
    for (int k = 0; k < K; k++) { busy[k] = s.busy[k]; pos[k] = s.pos[k]; }
    for (int t = s.turn + 1; t < T; t++) {
        for (int k = 0; k < K; k++) if (busy[k] > 0) busy[k]--;
        if (!sc.has[t]) continue;
        const int a = sc.a[t], b = sc.b[t];
        const int d = abs(a - b);
        const int horizon = T - t;                     // これ以上塞がっても損をしない
        int bk = -1, bw = INT32_MAX;
        for (int k = 0; k < K; k++) if (busy[k] == 0) {
            const int w = abs(pos[k] - a);             // 迎車距離が最小の空車 = 利益最大
            if (w < bw) { bw = w; bk = k; }
        }
        if (bk < 0) continue;
        const int   dur  = bw + d;
        const float v    = (float)d * (float)FARE - (float)bw * (float)MCOST;
        const float cost = ROLLOUT_TH * (float)(dur < horizon ? dur : horizon);
        if (v > cost) { total += v; busy[bk] = dur; pos[bk] = b; }
    }
    return total;
}

void read_input() {
    if (scanf("%d %d %d %d %d %d", &T, &K, &L, &FARE, &MCOST, &QREQ) == 6 && T >= 1) {
        T = min(T, MAXT); K = min(K, MAXK);
        for (int k = 0; k < K; k++) scanf("%d", &INIT_POS[k]);
        for (int t = 0; t < T; t++) scanf("%d %d %d", &TRUE_HAS[t], &TRUE_A[t], &TRUE_B[t]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 408);
        T = 200; K = 5; L = 100; FARE = 3; MCOST = 2; QREQ = 6000;
        for (int k = 0; k < K; k++) INIT_POS[k] = (int)g.next((uint32_t)(L + 1));
        for (int t = 0; t < T; t++) {
            TRUE_HAS[t] = ((int)g.next(10000) < QREQ) ? 1 : 0;
            TRUE_A[t] = (int)g.next((uint32_t)(L + 1));
            do { TRUE_B[t] = (int)g.next((uint32_t)(L + 1)); } while (TRUE_B[t] == TRUE_A[t]);
        }
    }
}

void output(const vector<Move> &hist) {
    string r; r.reserve(hist.size() * 3);
    for (const Move &mv : hist) { r += to_string((int)mv.car); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &hist) {
    float total = 0.0f;
    int busy[MAXK], pos[MAXK];
    for (int k = 0; k < K; k++) { busy[k] = 0; pos[k] = INIT_POS[k]; }
    for (int t = 0; t < T && t < (int)hist.size(); t++) {
        for (int k = 0; k < K; k++) if (busy[k] > 0) busy[k]--;
        const int c = hist[t].car;
        if (c < 0) continue;
        if (c >= K || !TRUE_HAS[t] || busy[c] > 0) { fprintf(stderr, "[error] 不正な配車 (t=%d)\n", t); return -1.0f; }
        total += (float)abs(TRUE_A[t] - TRUE_B[t]) * (float)FARE - (float)abs(pos[c] - TRUE_A[t]) * (float)MCOST;
        busy[c] = abs(pos[c] - TRUE_A[t]) + abs(TRUE_A[t] - TRUE_B[t]);
        pos[c] = TRUE_B[t];
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
