// =============================================================================
//  [貪欲系 08] ジョブ割当 (納期遅れペナルティ最小化)
// =============================================================================
//  【問題】
//    N 個のジョブと M 台のマシンがある。ジョブ i は仕事量 w_i、納期 d_i、遅延単価 p_i を持つ。
//    マシン k は速度 spd_k で、ジョブ i を単独で処理すると w_i / spd_k 時間かかる。
//    すべてのジョブをどれか 1 台に割り当て、各マシン内の処理順を決める。
//    マシンは割り当てられたジョブを順に処理し、ジョブ i の完了時刻を C_i とする。
//    ペナルティ Σ p_i * max(0, C_i - d_i) を最小化せよ。
//  【入力】
//    N M
//    spd_0 ... spd_{M-1}
//    w_i d_i p_i   (N 行)
//  【出力】
//    M 行。各行は "m j_1 ... j_m" (そのマシンが処理するジョブを処理順に)
//  【スコア】 総ペナルティ (小さいほど良い)。全ジョブがちょうど 1 回現れないと 0 点。
//  【入力生成方法】
//    N=200, M=4 固定。spd_k = 1.0 + 0.3*k。w_i は 10..100 の一様整数、p_i は 1..10 の一様整数。
//    d_i は [0, 1.2 * (Σw_i / Σspd_k)] の一様実数を四捨五入。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     この問題で決定的に効くのは「評価関数を正しく選ぶこと」。
//     遅延ペナルティ最小化のスケジューリングには ATC 則 (Apparent Tardiness Cost) という
//     よく知られた優先度規則があり、それを使うだけでスコアが 2 桁変わる。
//       優先度 = (p_i / 処理時間) × exp( - max(0, 納期 - 処理時間 - 現在時刻) / (KATC × 平均処理時間) )
//     ・遅れが確定している仕事は exp 項が 1 になり「単価 ÷ 処理時間が大きい順」(WSPT 則) になる
//     ・まだ余裕がある仕事は指数的に後回しになる
//     つまり「今やらないと損する順」に自然に並ぶ。
//
//   ● 状態 (State)
//     各マシンの現在時刻、ジョブごとの割当先、割り当てた順序。
//
//   ● 手の作り方
//     素朴には (未割当ジョブ × 全マシン) が候補だが、混雑したマシンにジョブが積み上がって破綻する
//     (実測で +44000% 悪化)。そこで候補を
//       「各ジョブについて、そのジョブが一番早く終わるマシン」1 つだけ
//     に絞る (最速完了則)。候補数が N 個になり、質も速度も上がる。
//
//   ● 評価値
//     上の ATC 優先度。最小化問題なので、内部では符号を反転して扱う。
//     KATC は「納期の余裕をどれくらい重く見るか」のパラメータで、0.45 が最良だった。
//
//   ● 差分計算 / 高速化
//     ジョブを 1 個置いたときの追加ペナルティは O(1)。
//     exp は毎手 N 回呼ぶので、精度を落とした近似式に置き換えてある。
//     ノイズ付きの貪欲を時間いっぱい繰り返す。
//
//   ● つまずきポイント
//     ・初版の評価値は「追加ペナルティ + ALPHA × 処理時間」で、納期の余裕をまったく見ていなかった。
//       その場のペナルティだけ見ると「まだ間に合う仕事」が延々と後回しにされ、
//       最後にまとめて大遅延する。これが 2 桁の差になった。
//     ・マシンを絞らないと「速いマシンに全部積む」動きになる。最速完了則との組み合わせが必須。
//
//   ● さらに伸ばすなら
//     ・挿入貪欲にして「マシン内の途中に差し込む」手を許す (ただし完了時刻が後続に波及するので
//       差分計算が O(そのマシンのジョブ数) になる。挿入位置を絞る必要がある)
//     ・焼きなましに移して「ジョブの移動 / 交換」を近傍にする
//
//  【改善】
//    評価値を「追加ペナルティ + ALPHA * 処理時間」から ATC 則 (納期の余裕で指数的に減衰させた
//    単価/処理時間) に変更し、マシンは「そのジョブが一番早く終わるマシン」だけを候補にした。
//    exp は近似式にして評価コストを抑え、KATC=0.45 / ノイズ 1.2 に調整した。
//    seed 0..4 合計: 変更前 593007 -> 変更後 3937 (-99.3%) ※seed 5..9 でも 709168 -> 7874 (-98.9%)
//
//  【採用したライブラリ】 greedy/1_greedy
//    実測比較 (seed 0,1,2 の合計スコア): 1_greedy=356585 / 3_rolling_horizon=459398
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
float ALPHA     = 0.0f;    // p1: マシンの現在時刻に対するペナルティ (負荷分散の強さ)
float EPS_DIST  = 1.0f;    // p2: 0 除算よけ
float NOISE     = 1.20f;   // p3: ランダム貪欲のノイズ幅 (0 なら決定的な貪欲 1 回)
float KATC      = 0.45f;    // p4: ATC 則の先読み幅 (大きいほど納期を軽視して WSPT 則に近づく)
int   MODE      = 2;       // p5: 1 = 一番早く空くマシンだけ / 2 = ジョブごとに最速完了マシン / 0 = 全組

void load_params() {
    pick_env("p1", ALPHA);
    pick_env("p2", EPS_DIST);
    pick_env("p3", NOISE);
    pick_env("p4", KATC);
    pick_env("p5", MODE);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
// constexpr bool MAXIMIZE = true;     // ← スコア最大化
constexpr bool MAXIMIZE = false;       // ← スコア最小化

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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (ジョブ割当)
// #############################################################################
constexpr int MAXN = 400, MAXM = 8;
constexpr int MAX_CAND = MAXN * MAXM;      // ★候補は (ジョブ × マシン)

int N, M;
static int   WJ[MAXN], DJ[MAXN], PJ[MAXN];
static float SPD[MAXM];
static float AVGP = 1.0f;                  // 平均処理時間 (ATC 則の先読み幅の基準)
static float IPT[MAXN][MAXM];              // 処理時間 w_i / spd_k の前計算
static float RATIO[MAXN][MAXM];            // p_i / 処理時間 (WSPT 則の指数) の前計算

struct Move { int16_t job; int8_t mac; };

struct State {
    float   score;             // 累積ペナルティ
    float   tm[MAXM];          // 各マシンの現在時刻
    int8_t  asg[MAXN];         // ジョブ -> マシン (-1 未割当)
    int16_t ord[MAXN];         // 割り当てた順
    int16_t done;
};

// exp(x) (x<=0) の高速近似。評価関数で毎回呼ぶので expf は使わない
static inline float fexp(float x) {
    if (x < -20.0f) return 0.0f;
    union { float f; int32_t i; } u;
    u.i = (int32_t)(12102203.0f * x + 1064866805.0f);
    return u.f;
}

void init_state(State &s) {
    s.score = 0.0f; s.done = 0;
    for (int k = 0; k < M; k++) s.tm[k] = 0.0f;
    memset(s.asg, -1, sizeof(int8_t) * (size_t)N);
}

// ★候補の絞り込み: MODE>0 なら「一番早く空くマシン」だけを候補にする (リストスケジューリング)
inline int enum_moves(const State &s, Move *out) {
    if (s.done >= N) return 0;
    int m = 0;
    if (MODE == 2) {                       // 各ジョブごとに「一番早く終わるマシン」だけを候補にする
        for (int i = 0; i < N; i++) if (s.asg[i] < 0) {
            int kb = 0; float cb = s.tm[0] + IPT[i][0];
            for (int k = 1; k < M; k++) { const float c = s.tm[k] + IPT[i][k]; if (c < cb) { cb = c; kb = k; } }
            out[m].job = (int16_t)i; out[m].mac = (int8_t)kb; m++;
        }
    } else if (MODE > 0) {
        int kb = 0;
        for (int k = 1; k < M; k++) if (s.tm[k] < s.tm[kb]) kb = k;
        for (int i = 0; i < N; i++) if (s.asg[i] < 0) { out[m].job = (int16_t)i; out[m].mac = (int8_t)kb; m++; }
    } else {
        for (int i = 0; i < N; i++) if (s.asg[i] < 0)
            for (int k = 0; k < M; k++) { out[m].job = (int16_t)i; out[m].mac = (int8_t)k; m++; }
    }
    return m;
}
inline float calc_score(const State &s, const Move &mv) {
    const float c = s.tm[mv.mac] + IPT[mv.job][mv.mac];
    return (float)PJ[mv.job] * max(0.0f, c - (float)DJ[mv.job]);
}
// 評価値: ATC 則 (Apparent Tardiness Cost)。
//   優先度 = (p_i / 処理時間) * exp( - max(0, 余裕) / (KATC * 平均処理時間) )
//   ・遅れが確定した仕事は exp 項が 1 になり WSPT 則 (単価/処理時間が大きい順) になる
//   ・まだ余裕がある仕事は指数的に後回しになる
//   最小化テンプレートなので符号を反転して返す。
inline float eval_move(const State &s, const Move &mv) {
    const float t  = IPT[mv.job][mv.mac];
    const float sl = (float)DJ[mv.job] - t - s.tm[mv.mac];
    const float e  = (sl > 0.0f) ? fexp(-sl / (KATC * AVGP)) : 1.0f;
    return -RATIO[mv.job][mv.mac] * e - ALPHA * s.tm[mv.mac];
}
inline void apply_move(State &s, const Move &mv) {
    s.tm[mv.mac] += IPT[mv.job][mv.mac];
    s.asg[mv.job] = mv.mac;
    s.ord[s.done++] = mv.job;
}

void read_input() {
    if (scanf("%d %d", &N, &M) == 2 && N >= 1) {
        N = min(N, MAXN); M = min(M, MAXM);
        for (int k = 0; k < M; k++) scanf("%f", &SPD[k]);
        for (int i = 0; i < N; i++) scanf("%d %d %d", &WJ[i], &DJ[i], &PJ[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 88);
        N = 200; M = 4;
        float tot = 0.0f, sp = 0.0f;
        for (int k = 0; k < M; k++) { SPD[k] = 1.0f + 0.3f * (float)k; sp += SPD[k]; }
        for (int i = 0; i < N; i++) { WJ[i] = 10 + (int)g.next(91); PJ[i] = 1 + (int)g.next(10); tot += (float)WJ[i]; }
        const float span = 1.2f * tot / sp;
        for (int i = 0; i < N; i++) DJ[i] = (int)lroundf(g.nextf() * span);
    }
    float sw = 0.0f, ss = 0.0f;
    for (int i = 0; i < N; i++) sw += (float)WJ[i];
    for (int k = 0; k < M; k++) ss += SPD[k];
    AVGP = (sw / (float)N) / (ss / (float)M);         // 平均処理時間
    for (int i = 0; i < N; i++) for (int k = 0; k < M; k++) {
        IPT[i][k]   = (float)WJ[i] / SPD[k];
        RATIO[i][k] = (float)PJ[i] / IPT[i][k];
    }
}

void output(const State &s) {
    for (int k = 0; k < M; k++) {
        string r = "";
        int c = 0;
        for (int t = 0; t < s.done; t++) if (s.asg[s.ord[t]] == k) { r += " "; r += to_string((int)s.ord[t]); c++; }
        printf("%d%s\n", c, r.c_str());
    }
}

float replay_true_score(const State &s) {
    if (s.done != N) { fprintf(stderr, "[error] 未割当のジョブがある\n"); return -1.0f; }
    static uint8_t seen[MAXN]; memset(seen, 0, sizeof(uint8_t) * (size_t)N);
    float tm[MAXM]; for (int k = 0; k < M; k++) tm[k] = 0.0f;
    float total = 0.0f;
    for (int t = 0; t < s.done; t++) {
        const int i = s.ord[t], k = s.asg[i];
        if (i < 0 || i >= N || seen[i] || k < 0 || k >= M) { fprintf(stderr, "[error] 不正な割当\n"); return -1.0f; }
        seen[i] = 1;
        tm[k] += (float)WJ[i] / SPD[k];
        total += (float)PJ[i] * max(0.0f, tm[k] - (float)DJ[i]);
    }
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
