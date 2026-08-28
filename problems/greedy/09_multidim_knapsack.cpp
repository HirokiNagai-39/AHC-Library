// =============================================================================
//  [貪欲系 09] 多次元ナップサック (資源制約付きアイテム選択)
// =============================================================================
//  【問題】
//    N 個のアイテムがある。アイテム i は D 種類の資源をそれぞれ a[i][d] だけ消費し、価値 v_i を持つ。
//    資源 d は全体で B_d までしか使えない。すべての資源制約を満たすようにアイテムを選び、
//    選んだアイテムの価値の合計を最大化せよ。
//  【入力】
//    N D
//    B_0 ... B_{D-1}
//    v_i a[i][0] ... a[i][D-1]   (N 行)
//  【出力】
//    N 個の 0/1 を 1 行に 1 つ (1 なら選ぶ)。
//  【スコア】 選んだアイテムの価値の合計 (大きいほど良い)。資源超過があれば 0 点。
//  【入力生成方法】
//    N=300, D=5 固定。a[i][d] は 1..100 の一様整数。
//    v_i = round(Σ_d a[i][d] / D * u), u は [0.5,1.5] の一様乱数。
//    B_d = round(0.3 * Σ_i a[i][d])。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     多次元ナップサックの定番は「残容量で正規化したコスパ」で選ぶ貪欲。
//     資源が 1 種類なら「価値 ÷ 重さ」で良いが、複数あると
//     「どの資源が今きついか」で同じアイテムの価値が変わる。
//     そこで消費量をその資源の残量で割ってから足し合わせると、
//     逼迫している資源を食うアイテムが自動的に嫌われる。
//
//   ● 状態 (State)
//     各資源の残容量と、アイテムごとの採用フラグ。
//
//   ● 手の作り方
//     「どの資源にもまだ入るアイテム」が候補。1 個ずつ入れていき、入るものが無くなったら終わり。
//
//   ● 評価値
//     v_i / ( Σ_d a[i][d] / (残容量_d + EPS) )^ALPHA
//     分母が「今の残容量から見た相対的な重さ」。序盤は全部余っているので価値重視、
//     終盤はきつい資源を食うアイテムが自然に敬遠される。
//     ALPHA でコスパの効かせ具合を、EPS で「残量がほぼ 0 の資源」の発散を抑える。
//
//   ● 差分計算 / 高速化
//     資源の残量を引くだけなので 1 手 O(D)。候補走査が O(N*D)。
//     ノイズ付きの貪欲を時間いっぱい繰り返して最良解を採用する。
//
//   ● つまずきポイント
//     ・残容量で割らずに固定の重み (例: Σa[i][d]) で正規化すると、
//       終盤に「もう入らない資源」を食うアイテムを選び続けてしまう。
//     ・powf を毎回呼ぶと重い。ALPHA が 1 のときは特別扱いすると試行数が数倍になる。
//
//   ● さらに伸ばすなら
//     ・この設定では既にほぼ厳密最適に到達している。
//       LP/MILP で上界を計算したところ seed 0: 解 6076 / 最適 6076〜6084、
//       seed 1: 解 6081 / 最適 6084、seed 2: 解 6120 / 最適 6124 で、残る伸びしろは 0.05% 程度。
//     ・伸ばすなら N や D を大きくして、ruin & recreate 型の反復局所探索を入れることになる
//       (この規模では試したが測定ノイズ内だった)
//
//  【採用したライブラリ】 greedy/1_greedy
//    実測比較 (seed 0,1,2 の合計スコア): 1_greedy=18277 / 3_rolling_horizon=17468
// =============================================================================
// =============================================================================
//  貪欲法 (Greedy) / ランダム多点貪欲   ---  AHC 用 高速テンプレート
// =============================================================================
//  毎回一番良い手を選んで解を伸ばす。評価値にノイズを掛けて時間いっぱい繰り返し最良解を採用する。
//
//  【貪欲法の 3 つの型】
//    1_greedy            … 毎回「一番良い手」を選んで解の末尾に足していく (最速・最も単純)
//    2_insertion_greedy  … 末尾だけでなく「操作列の途中」にも挿入できる貪欲 (過去改変貪欲)
//    3_rolling_horizon   … 各候補手について先を貪欲でプレイアウトし、結果が最良の手を選ぶ
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct Move  { ... };                     ... 手
//    struct State { float score; ... };        ... 状態 (score は必須)
//    init_state(State&)                        ... 初期状態
//    enum_moves(const State&, Move*)           ... 今選べる手を全列挙して個数を返す (0 で終了)
//    eval_move(const State&, Move)             ... 貪欲の評価値 (大きいほど先に選ぶ)
//    calc_score(const State&, Move)            ... その手で確定するスコア差分
//    apply_move(State&, Move)                  ... 手を適用する (score は触らない)
//
//  【高速化のポイント】
//    ・候補の評価は全ソートせず「最大のものを 1 パスで拾う」だけ (O(候補数))
//    ・State・候補配列はすべて静的確保。ループ中に malloc を一度も呼ばない
//    ・評価値にノイズを掛けたランダム貪欲を時間いっぱい繰り返し、最良解を採用する
//      (NOISE = 0 なら決定的な貪欲 1 回で終了する)
//    ・時間計測は rdtsc/cntvct 直読み。1 試行に 1 回だけ
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
float ALPHA     = 1.0f;    // p1: 評価値のコスト項の効かせ方
float EPS_DIST  = 1.0f;    // p2: 0 除算よけ
float NOISE     = 0.30f;   // p3: ランダム貪欲のノイズ幅 (0 なら決定的な貪欲 1 回)

void load_params() {
    pick_env("p1", ALPHA);
    pick_env("p2", EPS_DIST);
    pick_env("p3", NOISE);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化
// constexpr bool MAXIMIZE = false;    // ← スコア最小化

inline bool is_better(float a, float b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }
constexpr float WORST_SCORE = MAXIMIZE ? -3.0e38f : 3.0e38f;

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS = 1900.0f;   // 全体の時間制限[ms] (実行時間制限 - 余裕)
// (この手法は探索設定より「評価関数」と「1 手のコスト」が効くので、ここは少なめ)

// 統計 (デバッグ用。不要なら消して良い)
static ll g_trials = 0, g_steps = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (多次元ナップサック)
// #############################################################################
constexpr int MAXN = 600, MAXD = 8;
constexpr int MAX_CAND = MAXN;             // ★候補はアイテム全部

int N, DD;
static int AA[MAXN][MAXD], VV[MAXN], BB[MAXD];

struct Move { int16_t id; };

struct State {
    float   score;
    int32_t rest[MAXD];
    uint8_t take[MAXN];
};

void init_state(State &s) {
    s.score = 0.0f;
    for (int d = 0; d < DD; d++) s.rest[d] = BB[d];
    memset(s.take, 0, sizeof(uint8_t) * (size_t)N);
}

inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    for (int i = 0; i < N; i++) {
        if (s.take[i]) continue;
        bool ok = true;
        for (int d = 0; d < DD; d++) if (AA[i][d] > s.rest[d]) { ok = false; break; }
        if (ok) out[m++].id = (int16_t)i;
    }
    return m;
}
inline float calc_score(const State &s, const Move &mv) { (void)s; return (float)VV[mv.id]; }
// 評価値: 価値 / (残容量で正規化した資源消費の合計)^ALPHA
//   … 「今きつい資源をたくさん使うアイテム」を自動的に嫌う定番のコスパ指標
inline float eval_move(const State &s, const Move &mv) {
    float cost = 0.0f;
    for (int d = 0; d < DD; d++) cost += (float)AA[mv.id][d] / ((float)s.rest[d] + EPS_DIST);
    return (float)VV[mv.id] / powf(cost + 1e-6f, ALPHA);
}
inline void apply_move(State &s, const Move &mv) {
    for (int d = 0; d < DD; d++) s.rest[d] -= AA[mv.id][d];
    s.take[mv.id] = 1;
}

void read_input() {
    if (scanf("%d %d", &N, &DD) == 2 && N >= 1) {
        N = min(N, MAXN); DD = min(DD, MAXD);
        for (int d = 0; d < DD; d++) scanf("%d", &BB[d]);
        for (int i = 0; i < N; i++) { scanf("%d", &VV[i]); for (int d = 0; d < DD; d++) scanf("%d", &AA[i][d]); }
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 99);
        N = 300; DD = 5;
        long long sum[MAXD] = {0};
        for (int i = 0; i < N; i++) {
            int t = 0;
            for (int d = 0; d < DD; d++) { AA[i][d] = 1 + (int)g.next(100); sum[d] += AA[i][d]; t += AA[i][d]; }
            VV[i] = 1 + (int)lroundf((float)t / (float)DD * (0.5f + g.nextf()));
        }
        for (int d = 0; d < DD; d++) BB[d] = (int)(0.3 * (double)sum[d]);
    }
}

void output(const State &s) {
    string r; r.reserve((size_t)N * 2);
    for (int i = 0; i < N; i++) { r += (s.take[i] ? '1' : '0'); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const State &s) {
    long long use[MAXD] = {0};
    float total = 0.0f;
    for (int i = 0; i < N; i++) if (s.take[i]) {
        total += (float)VV[i];
        for (int d = 0; d < DD; d++) use[d] += AA[i][d];
    }
    for (int d = 0; d < DD; d++) if (use[d] > BB[d]) { fprintf(stderr, "[error] 資源 %d 超過\n", d); return -1.0f; }
    return total;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
// 7. 貪欲法 本体
// =============================================================================
//  1 回の貪欲 = 「候補を全部評価して一番良い手を選ぶ」を手が無くなるまで繰り返す。
//  NOISE > 0 なら評価値にランダムな倍率を掛けて何度も貪欲を回し、最良解を採用する
//  (ランダム多点貪欲)。局所的な決定ミスを引き直せるので、ほぼ確実に得をする。
static Move  g_cand[MAX_CAND];
static State g_cur, g_best;

// 1 回ぶんの貪欲を回す
static void run_greedy(State &s, float noise) {
    init_state(s);
    while (true) {
        const int m = enum_moves(s, g_cand);
        if (m == 0) break;
        // ---- 一番評価値が高い手を 1 パスで拾う (ソートしない) ----
        int   bi = -1;
        float bv = WORST_SCORE;
        for (int k = 0; k < m; k++) {
            float e = eval_move(s, g_cand[k]);
            if (e == WORST_SCORE) continue;              // 実行不可の手
            if (noise > 0.0f) e *= 1.0f + noise * (rng.nextf() - 0.5f);
            if (is_better(e, bv)) { bv = e; bi = k; }
        }
        if (bi < 0) break;
        s.score += calc_score(s, g_cand[bi]);
        apply_move(s, g_cand[bi]);
        g_steps++;
    }
    g_trials++;
}

void greedy(float deadline_ms) {
    run_greedy(g_best, 0.0f);                    // まず決定的な貪欲を 1 回
    if (NOISE <= 0.0f) return;                   // ノイズ無しなら繰り返す意味が無い
    while (timer.ms() < deadline_ms) {           // 時間いっぱいランダム貪欲を繰り返す
        run_greedy(g_cur, NOISE);
        if (is_better(g_cur.score, g_best.score)) g_best = g_cur;
    }
}

// =============================================================================
// 8. main
// =============================================================================
int main() {
    timer.start();      // ★ 一番最初にタイマー開始
    load_params();      // 環境変数からパラメータを読む (optuna 用)
    read_input();

    greedy(TIME_LIMIT_MS);
    output(g_best);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)replay_true_score(g_best));
    fprintf(stderr, "trials = %lld, steps = %lld, time = %.1f ms\n", g_trials, g_steps, (double)timer.ms());
    // ★実装のバグ検出: 積み上げたスコアと、出力を再生した真のスコアが一致するはず。
    //   貪欲は「選んだ手」でしか calc_score を呼ばないので、差分の誤りは必ずここに出る
    //   (焼きなましのように「却下された手の差分が間違っている」という隠れ方をしない)。
    //   なお eval_move は順位付け用のヒューリスティックなので、ここでは検査しない/する必要も無い。
    fprintf(stderr, "[check] running = %.0f / replay = %.0f\n",
            (double)g_best.score, (double)replay_true_score(g_best));
    return 0;
}
