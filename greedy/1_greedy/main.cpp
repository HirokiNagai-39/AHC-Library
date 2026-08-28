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
float ALPHA     = 1.0f;    // p1: 評価値の距離の効かせ方 (価値 / 距離^ALPHA)。問題ごとの評価関数の重み
float EPS_DIST  = 1.0f;    // p2: 0 除算よけ。小さいほど「ほぼ 0 距離」を極端に優先する
float NOISE     = 0.30f;   // p3: 評価値に掛けるランダムノイズの幅 (0 なら決定的な貪欲 1 回で終了)

void load_params() {
    pick_env("p1", ALPHA);
    pick_env("p2", EPS_DIST);
    pick_env("p3", NOISE);
}
// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化のとき こちらを有効化 (デモは最大化)
// constexpr bool MAXIMIZE = false;    // ← スコア最小化のとき こちらを有効化

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
// # 6. ■ ここから 問題ごとに書き換える部分 ■
// #     (デモ: 価値付き巡回 / orienteering)
// #       N 個の地点があり、地点 i には価値 v[i] がある (地点 0 は出発地で価値 0)。
// #       地点 0 から出発して地点 0 に戻る経路のうち、総移動距離が LIMIT 以下のものを選び、
// #       訪問した地点の価値の合計を最大化する。
// #       標準入力が無ければランダムに問題を生成するので、そのまま実行できる。
// #############################################################################
constexpr int MAXN = 400;             // 地点数の上限
constexpr int MAX_CAND = MAXN;        // ★1 手で選べる候補の最大数 (配列サイズに効く)
int   N;                              // 地点数
float LIMIT;                          // 総移動距離の上限
static int   VAL[MAXN];               // 各地点の価値
static float D[MAXN * MAXN];          // 距離行列 (i*N+j)
static inline float dist(int i, int j) { return D[i * N + j]; }

// ★必須: 手 = 「次に訪問する地点」
struct Move { int16_t to; };

// ★必須: 状態。float score を必ず持たせる。
struct State {
    float   score;                    // ★必須: 集めた価値の合計
    float   used;                     // 使った移動距離 (最後に 0 へ戻る分は含まない)
    int16_t last;                     // 現在地
    int16_t len;                      // 訪問した地点数
    int16_t seq[MAXN];                // 訪問順
    uint8_t vis[MAXN];                // 訪問済みフラグ
};

// ★必須: 初期状態
void init_state(State &s) {
    s.score = 0.0f; s.used = 0.0f; s.last = 0; s.len = 0;
    memset(s.vis, 0, sizeof(uint8_t) * N);
    s.vis[0] = 1;
}

// ★必須: 今選べる手を全部 out[] に詰めて個数を返す (0 を返したら終了)
inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    const float base = s.used;
    for (int i = 1; i < N; i++) {
        if (s.vis[i]) continue;
        if (base + dist(s.last, i) + dist(i, 0) <= LIMIT) out[m++].to = (int16_t)i;
    }
    return m;
}

// ★必須: その手で確定するスコア差分
inline float calc_score(const State &s, const Move &mv) { (void)s; return (float)VAL[mv.to]; }

// ★必須: 貪欲の評価値 (大きいほど先に選ぶ)。スコア差分と違って良い。
//   デモ: 価値 / (増える距離)^ALPHA  … 「安く高い価値が取れる地点」を優先する
inline float eval_move(const State &s, const Move &mv) {
    const int i = mv.to;
    const float add = dist(s.last, i) + dist(i, 0) - dist(s.last, 0);
    return (float)VAL[i] / powf(add + EPS_DIST, ALPHA);
}

// ★必須: 手を適用する (score は触らない。フレームワークが calc_score を足す)
inline void apply_move(State &s, const Move &mv) {
    s.used += dist(s.last, mv.to);
    s.last = mv.to;
    s.vis[mv.to] = 1;
    s.seq[s.len++] = mv.to;
}

// 入力 (デモ: 標準入力が無ければランダム生成)
void read_input() {
    static float px[MAXN], py[MAXN];
    if (scanf("%d %f", &N, &LIMIT) == 2 && N >= 2) {
        N = min(N, MAXN);
        for (int i = 0; i < N; i++) scanf("%f %f %d", &px[i], &py[i], &VAL[i]);
    } else {
        N = 200; LIMIT = 4000.0f;
        Xor128 g; g.seed(20260824);
        for (int i = 0; i < N; i++) { px[i] = g.nextf() * 1000.0f; py[i] = g.nextf() * 1000.0f; VAL[i] = 1 + (int)g.next(100); }
        VAL[0] = 0;
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            const float dx = px[i] - px[j], dy = py[i] - py[j];
            D[i * N + j] = sqrtf(dx * dx + dy * dy);
        }
}

// 出力 (訪問順。0 から始まり 0 で終わる)
void output(const State &s) {
    printf("0");
    for (int i = 0; i < s.len; i++) printf(" %d", (int)s.seq[i]);
    printf(" 0\n");
}

// 出力を再生して「真のスコア」を計算する (実装のバグ検出用)
float replay_true_score(const State &s) {
    float used = 0.0f, total = 0.0f;
    int prev = 0;
    static uint8_t seen[MAXN];
    memset(seen, 0, sizeof(uint8_t) * N);
    for (int i = 0; i < s.len; i++) {
        const int c = s.seq[i];
        if (c <= 0 || c >= N || seen[c]) { fprintf(stderr, "[error] 不正な訪問 %d\n", c); return -1.0f; }
        seen[c] = 1;
        used += dist(prev, c);
        total += (float)VAL[c];
        prev = c;
    }
    used += dist(prev, 0);
    if (used > LIMIT + 1e-3f) { fprintf(stderr, "[error] 距離超過 %.1f > %.1f\n", used, LIMIT); return -1.0f; }
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
