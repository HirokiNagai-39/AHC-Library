// =============================================================================
//  [焼きなまし系 03] 数値の格子配置 (行・列・対角の和を揃える)
// =============================================================================
//  【問題】
//    N×N のマスに、与えられた N^2 個の整数を 1 つずつ重複なく配置する。
//    各行の和、各列の和、2 本の対角線の和 が、目標値 T = (全整数の総和) / N に近くなるようにしたい。
//    ずれの合計 Σ|行和 - T| + Σ|列和 - T| + |対角和1 - T| + |対角和2 - T| を最小化せよ。
//  【入力】
//    N
//    v_0 ... v_{N^2-1}   (配置する整数)
//  【出力】
//    N 行 N 列の整数 (空白区切り)。
//  【スコア】 上記のずれの合計 (小さいほど良い)。与えられた整数が 1 つずつ使われていなければ 0 点。
//  【入力生成方法】
//    N=20 固定。v は 1..1000 の一様整数を N^2 個 (重複あり)。
//    値がばらばらなので、ぴったり揃えることは普通できない。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     1..N^2 ではなくランダムな整数を並べるので、ぴったり揃えることは普通できない。
//     「ずれの合計をどこまで小さくできるか」を competing する最小化問題。
//     近傍が 2 マスの値の入れ替えなら、値の多重集合は自動的に保たれる (重複や欠落が起きない)。
//     この問題の特徴は「1 回の探索が 0.1 秒ほどで完全に収束してしまう」こと。
//     残り 95% の時間が無駄になるので、初期配置を変えて何度もやり直す形にするのが正解。
//
//   ● 状態 (State)
//     盤面 a[] に加えて、各行の和 rs[]、各列の和 cs[]、2 本の対角和 d1, d2。
//     和を持っておくのが差分計算の肝。
//
//   ● 手の作り方
//     2 マスを選んで値を入れ替える。確率 P_NB で「同じ行の 2 マス」を選ぶと、
//     行和が変わらないので列だけを直す動きになる (狙いを絞った近傍)。
//     残りはランダムな 2 マス。
//
//   ● 評価値
//     Σ|行和 - T| + Σ|列和 - T| + |対角和1 - T| + |対角和2 - T| (T は目標値)。最小化。
//
//   ● 差分計算 / 高速化
//     入れ替えで変わるのは高々 2 行 2 列 2 対角なので、
//     「その 6 本ぶんの |和 - T| を入れ替え前後で計算して引く」だけの O(1)。
//     同じ行や同じ列を選んだ場合の重複に注意して場合分けする。
//
//   ● つまずきポイント
//     ・行/列/対角が重なるケース (同じ行を選んだ、対角線上のマスを選んだ) の場合分けを
//       間違えやすい。ここは [verify] が 1 手ずつ全計算と突き合わせるので必ず捕まる。
//     ・低温すぎると山登りと同じで 0.1 秒で終わる。「収束が速い問題は多点スタート」と覚えておくとよい。
//
//   ● さらに伸ばすなら
//     ・「一番ずれている行・列を狙って直す」貪欲な近傍を試したが、決定的すぎてすぐ詰まり
//       大幅に悪化した (16 -> 215)。狙い撃ちは確率を混ぜないと使えない。
//     ・現在のスコアは「行和が整数」という制約から決まる下限 (seed 0-4 で 56) にほぼ届いている
//
//  【改善】
//    山登りは 0.1 秒で収束しきっており残り 95% の時間が無駄だったため、多点スタート焼きなまし
//    (開始温度 0.3 / 終了温度 0.01 / 16 回スタート) に載せ替えた。低温なので中身はほぼ山登りだが、
//    初期配置を変えて 16 回やり直し最良を採るぶん、届く局所解の質が上がる。
//    seed 0..4 合計: 変更前 62 -> 変更後 57 (-8.1%)   ※seed 0..9 では 139 -> 127 (-8.6%)
//    (この問題は「行和・列和が整数」という制約から下限が決まっていて、その下限にほぼ届いている)
//
//  【採用したライブラリ】 simulated annealing/4_multi_start_annealing
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=42 / 2_hill_climbing_kick=64 / 3_simulated_annealing=73 / 4_multi_start_annealing=75 / 5_iterated_annealing=43
//    ※上は既定パラメータでの比較。温度を 0.3->0.01 まで下げてスタート回数を増やすと 4 が最良になる。
// =============================================================================
// =============================================================================
//  多点スタート焼きなまし (Multi-start SA)   ---  AHC 用 高速テンプレート
// =============================================================================
//  時間を等分し、毎回ちがう初期解から焼きなまして最良解を採用する。
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct State { float score; ... };  ... 状態 (float score は必須)
//    init_state(State&)   ... 初期解構築 (state.score も必ずセットする)
//    modify(State&)       ... 遷移(近傍)を 1 つ選ぶ ★この時点では state を変えない
//    calc_score(State&)   ... 選んだ遷移を適用したときの「スコア差分」を返す
//    apply_move(State&)   ... 採用が決まった遷移を実際に state へ反映する
//
//  【なぜ「選ぶ → 差分計算 → 採用時だけ適用」なのか】
//    山登り/焼きなましでは提案の大半が却下される。却下時に何もしないこの順序が最速。
//    「先に適用して却下時に戻す」方式にしたい場合は
//      ・modify() の中で適用まで行う
//      ・apply_move() を空にする
//      ・ループ中の "// else rollback(state);" のコメントを外し rollback() を書く
//
//  【高速化のポイント】
//    ・乱数は xorshift128 (剰余を使わない [0,n) 生成)
//    ・時間計測は rdtsc(x86) / cntvct(ARM) を直接読む (chrono の ~1/10 のコスト)
//    ・時間計測は ITER_PER_CHECK 回に 1 回だけ (内側ループには時間計測が一切無い)
//    ・exp() を使わず log(乱数) のテーブル比較で採用判定 (焼きなまし系)
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
    // 32bit 乱数
    inline uint32_t next() {
        uint32_t t = x ^ (x << 11);
        x = y; y = z; z = w;
        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    }
    // [0, n) の整数 (剰余 % を使わないので高速)
    inline uint32_t next(uint32_t n) { return (uint32_t)(((uint64_t)next() * n) >> 32); }
    // [l, r) の整数
    inline uint32_t next(uint32_t l, uint32_t r) { return l + next(r - l); }
    // [0, 1) の float
    inline float nextf() { return (float)(next() >> 8) * (1.0f / 16777216.0f); }
    // [0, 1) の double
    inline double nextd() { return (double)next() * (1.0 / 4294967296.0); }
};
static Xor128 rng;

// xorshift による高速シャッフル
template <class T> inline void rnd_shuffle(T *a, int n) {
    for (int i = n - 1; i > 0; i--) swap(a[i], a[rng.next(i + 1)]);
}

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
        ms_per_tick_ = 1000.0 / (double)f; calib_ = 100;   // ARM は周波数が正確に取れる
#elif defined(__x86_64__) || defined(_M_X64)
        ms_per_tick_ = 1.0 / 2.8e6; calib_ = 0;            // 仮値(2.8GHz)。下で自動補正する
#else
        ms_per_tick_ = 1e-6; calib_ = 100;                 // chrono(ns) はそのまま
#endif
        w0_ = chrono::steady_clock::now();
        t0_ = tick();
    }
    // 経過時間[ms] (ホットループから呼ぶのはこれ)
    inline float ms() {
        uint64_t d = tick() - t0_;
        if (calib_ < 4) calibrate(d);       // 最初の数回だけ実時計で周波数を補正 (誤差 <0.01%)
        return (float)((double)d * ms_per_tick_);
    }
    void calibrate(uint64_t d) {
        double w = chrono::duration<double, milli>(chrono::steady_clock::now() - w0_).count();
        if (w > 0.25 * (double)(1u << (2 * calib_))) {      // 0.25ms, 1ms, 4ms, 16ms 時点で補正
            ms_per_tick_ = w / (double)d;
            calib_++;
        }
    }
};
static Timer timer;

// =============================================================================
// 3. パラメータ (optuna から環境変数 p1, p2, ... で上書きできる)
// =============================================================================
// optimize.py の generate_params() のキーと対応させる。
// 環境変数が無ければ既定値が使われるので、そのまま提出して良い。
template <class T> inline void pick_env(const char *key, T &dst) {
    if (const char *s = getenv(key)) {
        char *e = nullptr;
        double v = strtod(s, &e);
        if (e != s) dst = (T)v;
    }
}

// ---- 調整パラメータ (p1, p2, ... が optimize.py のキーと対応) ----
float TEMP_START = 0.3f;           // p1: 開始温度
float TEMP_END = 0.01f;            // p2: 終了温度
int   NUM_STARTS = 16;             // p3: スタート回数
float P_NB = 0.5f;                 // p4: 同じ行の中で入れ替える確率

void load_params() {
    pick_env("p1", TEMP_START);
    pick_env("p2", TEMP_END);
    pick_env("p3", NUM_STARTS);
    pick_env("p4", P_NB);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
// constexpr bool MAXIMIZE = true;     // ← スコア最大化
constexpr bool MAXIMIZE = false;       // ← スコア最小化

// gain (改善量) = GAIN_SIGN * スコア差分。 gain > 0 なら「良くなった」
constexpr float GAIN_SIGN = MAXIMIZE ? 1.0f : -1.0f;
// a が b より良ければ true
inline bool is_better(float a, float b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS   = 1900.0f;  // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr int   ITER_PER_CHECK  = 1024;     // ★ 時間計測はこの回数に 1 回だけ行う
constexpr bool  ADAPTIVE_BLOCK  = true;     // 1 ブロックの実行時間が一定になるよう回数を自動調整
constexpr float TARGET_BLOCK_MS = 0.5f;     // ADAPTIVE_BLOCK=true のときの 1 ブロックの目標時間[ms]

// ---- 焼きなまし用 ----
constexpr bool  KEEP_BEST      = true;   // 最良解を別に保持する (State のコピーが重いなら false)
constexpr float KEEP_BEST_FROM = 0.5f;   // 進捗率がこれを超えてから記録開始 (序盤の無駄なコピーを削減)

// 1 ブロック(= 時間計測 1 回分)の反復回数を調整する。
// 近傍が重い問題でも時間超過せず、軽い問題では時間計測の回数を減らせる。
inline void tune_block(int &block, float dt) {
    if constexpr (ADAPTIVE_BLOCK) {
        if (dt > 1e-4f) {
            const double nb = (double)block * ((double)TARGET_BLOCK_MS / (double)dt);
            block = (int)min(max(nb, 32.0), 1.0e7);
        }
    }
}

// 統計 (デバッグ用。不要なら消して良い)
static ll g_iter = 0, g_accept = 0;
static ll g_round = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (魔方陣風の数値配置)
// #############################################################################
constexpr int MAXN = 40;

int N;
static double TGT;
static int VS[MAXN * MAXN];

struct State {
    float   score;                  // ★必須: ずれの合計
    int32_t a[MAXN * MAXN];
    long long rs[MAXN], cs[MAXN], d1, d2;
};

struct Move { int p, q; };          // マス p と q の値を入れ替える
static Move g_mv;

static inline float dev(long long x) { return (float)fabs((double)x - TGT); }

float full_score(const State &s) {
    long long rs[MAXN] = {0}, cs[MAXN] = {0}, d1 = 0, d2 = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            const long long v = s.a[i * N + j];
            rs[i] += v; cs[j] += v;
            if (i == j) d1 += v;
            if (i + j == N - 1) d2 += v;
        }
    float t = dev(d1) + dev(d2);
    for (int i = 0; i < N; i++) t += dev(rs[i]) + dev(cs[i]);
    return t;
}

void init_state(State &s) {
    const int M = N * N;
    for (int i = 0; i < M; i++) s.a[i] = VS[i];
    for (int i = M - 1; i > 0; i--) swap(s.a[i], s.a[rng.next(i + 1)]);
    for (int i = 0; i < N; i++) { s.rs[i] = 0; s.cs[i] = 0; }
    s.d1 = 0; s.d2 = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            const long long v = s.a[i * N + j];
            s.rs[i] += v; s.cs[j] += v;
            if (i == j) s.d1 += v;
            if (i + j == N - 1) s.d2 += v;
        }
    s.score = full_score(s);
}

inline void modify(State &s) {
    (void)s;
    const int M = N * N;
    g_mv.p = (int)rng.next((uint32_t)M);
    if (rng.nextf() < P_NB) {                       // 同じ行の中で入れ替える (行和は変わらない)
        g_mv.q = (g_mv.p / N) * N + (int)rng.next((uint32_t)N);
    } else {
        g_mv.q = (int)rng.next((uint32_t)M);
    }
}

inline float calc_score(State &s) {
    const int p = g_mv.p, q = g_mv.q;
    if (p == q) return 0.0f;
    const int r1 = p / N, c1 = p % N, r2 = q / N, c2 = q % N;
    const long long v1 = s.a[p], v2 = s.a[q], dv = v2 - v1;
    if (dv == 0) return 0.0f;
    float before = 0.0f, after = 0.0f;
    long long rs1 = s.rs[r1], rs2 = s.rs[r2], cs1 = s.cs[c1], cs2 = s.cs[c2], dd1 = s.d1, dd2 = s.d2;
    before += dev(rs1) + dev(cs1) + ((r1 == r2) ? 0.0f : dev(rs2)) + ((c1 == c2) ? 0.0f : dev(cs2));
    const bool p1 = (r1 == c1), q1 = (r2 == c2), p2 = (r1 + c1 == N - 1), q2 = (r2 + c2 == N - 1);
    if (p1 || q1) before += dev(dd1);
    if (p2 || q2) before += dev(dd2);
    if (r1 != r2) { rs1 += dv; rs2 -= dv; }
    if (c1 != c2) { cs1 += dv; cs2 -= dv; }
    if (p1) dd1 += dv;
    if (q1) dd1 -= dv;
    if (p2) dd2 += dv;
    if (q2) dd2 -= dv;
    after += dev(rs1) + dev(cs1) + ((r1 == r2) ? 0.0f : dev(rs2)) + ((c1 == c2) ? 0.0f : dev(cs2));
    if (p1 || q1) after += dev(dd1);
    if (p2 || q2) after += dev(dd2);
    return after - before;
}

inline void apply_move(State &s) {
    const int p = g_mv.p, q = g_mv.q;
    const int r1 = p / N, c1 = p % N, r2 = q / N, c2 = q % N;
    const long long dv = (long long)s.a[q] - (long long)s.a[p];
    if (r1 != r2) { s.rs[r1] += dv; s.rs[r2] -= dv; }
    if (c1 != c2) { s.cs[c1] += dv; s.cs[c2] -= dv; }
    if (r1 == c1) s.d1 += dv;
    if (r2 == c2) s.d1 -= dv;
    if (r1 + c1 == N - 1) s.d2 += dv;
    if (r2 + c2 == N - 1) s.d2 -= dv;
    swap(s.a[p], s.a[q]);
}

void read_input() {
    if (scanf("%d", &N) == 1 && N >= 2) {
        N = min(N, MAXN);
        for (int i = 0; i < N * N; i++) scanf("%d", &VS[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 203);
        N = 20;
        for (int i = 0; i < N * N; i++) VS[i] = 1 + (int)g.next(1000);
    }
    long long tot = 0;
    for (int i = 0; i < N * N; i++) tot += VS[i];
    TGT = (double)tot / (double)N;
}

void output(const State &s) {
    string r; r.reserve((size_t)N * N * 5);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) { if (j) r += ' '; r += to_string(s.a[i * N + j]); }
        r += '\n';
    }
    fputs(r.c_str(), stdout);
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
//  採用判定用: log(一様乱数) のテーブル
// =============================================================================
//  採用条件  exp(gain / T) > u   (u は [0,1) の一様乱数)
//         ⇔ gain > T * log(u)   (log(u) <= 0)
//  この形にすると exp も log も除算も要らず「1 回の乗算と比較」だけで判定できる。
//  さらに gain >= 0 (改善) のときは右辺が負なので必ず真になり、分岐も 1 つで済む。
constexpr int      LOG_TBL_BITS = 12;
constexpr int      LOG_TBL_SIZE = 1 << LOG_TBL_BITS;      // 4096 個 = 16KB (L1 に載る)
constexpr uint32_t LOG_TBL_MASK = LOG_TBL_SIZE - 1;
static float g_log_tbl[LOG_TBL_SIZE];
void init_log_table() {
    Xor128 g; g.seed(1234567);
    for (int i = 0; i < LOG_TBL_SIZE; i++)
        g_log_tbl[i] = logf(((float)g.next() + 0.5f) * (1.0f / 4294967296.0f));
}

// =============================================================================
// 7. 焼きなまし法 本体
// =============================================================================
//  s を初期解として受け取り、deadline_ms まで焼きなまし、見つけた最良解を s に残す。
//  t0 : 開始温度 (これくらいの悪化なら受け入れる、という値。差分の最大値くらい)
//  t1 : 終了温度 (これ以上の悪化は受け入れない、という値。差分の最小値くらい)
void anneal(State &s, float deadline_ms, float t0, float t1) {
    static State best;                       // State が大きくても良いように static 領域へ
    if constexpr (KEEP_BEST) best = s;

    const float begin = timer.ms();
    const float span  = deadline_ms - begin;
    if (span <= 0.0f) return;
    if (t0 < 1e-9f) t0 = 1e-9f;              // 0 除算 / NaN 対策
    if (t1 < 1e-9f) t1 = 1e-9f;
    if (t1 > t0)    t1 = t0;                 // 念のため t0 >= t1 にしておく
    const float lr = logf(t1 / t0);          // 指数スケジュール用

    float temp    = t0;
    bool  keeping = (KEEP_BEST_FROM <= 0.0f);
    int   block   = ITER_PER_CHECK;          // ★ この回数に 1 回だけ時間計測する
    float prev    = begin;

    while (true) {
        // ---- 内側ループ: 時間計測も温度更新も一切しない (ここが最速であるべき) ----
        for (int it = 0; it < block; it++) {
            modify(s);                                  // 遷移を 1 つ選ぶ
            const float diff = calc_score(s);           // スコア差分
            const float gain = GAIN_SIGN * diff;        // 改善量 (正なら改善)
            // gain > T*log(u) : 改善なら必ず採用、悪化なら exp(gain/T) の確率で採用
            if (gain > temp * g_log_tbl[rng.next() & LOG_TBL_MASK]) {
                apply_move(s);                          // 採用時だけ実際に反映
                s.score += diff;
                g_accept++;
                if constexpr (KEEP_BEST) {
                    if (keeping && is_better(s.score, best.score)) best = s;
                }
            }
            // else rollback(s);   // ←「先に適用して却下時に戻す」方式にする場合はここ
        }
        g_iter += block;

        // ---- ここからブロックごとの処理 (時間計測・温度更新) ----
        const float now = timer.ms();
        if (now >= deadline_ms) break;
        const float r = (now - begin) / span;           // 進捗 [0,1)
        temp = t0 * expf(lr * r);                       // 指数スケジュール (既定)
        // temp = t0 + (t1 - t0) * r;                   // 線形スケジュール (こちらが良い問題もある)

        if constexpr (KEEP_BEST) {
            if (!keeping && r >= KEEP_BEST_FROM) {      // 途中から最良解の記録を始める (コピー削減)
                keeping = true;
                if (is_better(s.score, best.score)) best = s;
            }
        }
        tune_block(block, now - prev); prev = now;      // 1 ブロックの実行時間を一定に保つ
    }
    if constexpr (KEEP_BEST) { if (is_better(best.score, s.score)) s = best; }
}

// =============================================================================
// 7-2. 多点スタート焼きなまし 本体
// =============================================================================
//  時間を NUM_STARTS 等分し、毎回ちがう初期解から焼きなまして最良解を採用する。
//  1 回あたりの時間は短くなるが、初期解によって結果が大きくぶれる問題や、
//  局所解が多くて 1 回の焼きなましでは抜けられない問題に強い。
void multi_start_annealing(State &s, float tl_ms) {
    static State best;
    bool has_best = false;
    const float begin = timer.ms();
    const float span  = (tl_ms - begin) / (float)max(1, NUM_STARTS);
    for (int k = 0; k < NUM_STARTS; k++) {
        const float deadline = begin + span * (float)(k + 1);
        init_state(s);                                   // ★ 毎回ちがう初期解から
        anneal(s, deadline, TEMP_START, TEMP_END);
        if (!has_best || is_better(s.score, best.score)) { best = s; has_best = true; }
        g_round++;
        if (timer.ms() >= tl_ms) break;
    }
    s = best;
}

// =============================================================================
//  ★差分計算の自動検証 (main の最初に走る)
// =============================================================================
//  下の [check] は「最後に積み上げたスコア」と「全計算したスコア」を比べるだけなので、
//    ・却下された手の calc_score が間違っている (焼きなましでは大半の手が却下される)
//    ・apply_move が s.score を上書きしていて差分の誤りが打ち消されている
//    ・誤差が偶然打ち消し合っている
//  といったバグを見逃す。これらは「探索が弱くなるだけで最後の値は合う」ので特に気付きにくい。
//
//  そこでここでは 1 手ずつ
//     「calc_score() の返り値」 対 「実際に apply_move して full_score() を取り直した変化量」
//  を突き合わせる。採用・却下に関係なく全部の手を検査するので、上の 3 つとも捕まえられる。
//
//  ※ 検証中は「必ず手を適用して先へ進む」。State の外 (グローバル変数) に差分計算用の情報を
//     持つ実装だと、状態だけ巻き戻しても整合が取れず誤検出になるため。
//     ランダムウォークになるので、初期解の周りだけでなく色々な状態を通る。
//  ※ 乱数の状態は前後で復元するので、VERIFY_MOVES を変えても探索結果は 1 ビットも変わらない。
//  ※ 提出時に消したければ VERIFY_MOVES = 0 にする (数 ms しか掛からないので普段は付けたままで良い)。
constexpr int VERIFY_MOVES = 200;

static void verify_diff() {
    if constexpr (VERIFY_MOVES <= 0) return;
    const Xor128 save = rng;                       // 乱数列を汚さないよう退避
    static State s;
    init_state(s);
    int bad = 0;
    for (int i = 0; i < VERIFY_MOVES; i++) {
        const float before = full_score(s);
        modify(s);
        const float diff = calc_score(s);
        apply_move(s);                             // ★必ず適用して先へ進む (下の注意を参照)
        s.score += diff;                           // フレームワークと同じ更新をする
        const float after = full_score(s);
        const float act = after - before;          // 実際に起きたスコアの変化
        // 許容誤差は「全計算の float 丸め (合計値に比例)」+「差分そのものの 0.1%」。
        // 合計値だけを基準にすると、合計が大きい問題で差分の誤りが埋もれてしまう。
        const float tol = 1e-4f * max(1.0f, max(fabsf(before), fabsf(after)))
                        + 1e-3f * max(fabsf(diff), fabsf(act));
        if (fabsf(act - diff) > tol) {
            if (++bad <= 5)
                fprintf(stderr, "[verify] NG %d 手目: calc_score=%.4f / 実際の変化=%.4f (ずれ %.4f)\n",
                        i, (double)diff, (double)act, (double)(act - diff));
        }
    }
    if (bad) fprintf(stderr, "[verify] ★差分計算が %d/%d 手でずれています。calc_score か apply_move にバグがあります\n", bad, VERIFY_MOVES);
    else     fprintf(stderr, "[verify] 差分計算 OK (%d 手を全計算と突き合わせ)\n", VERIFY_MOVES);
    init_state(s);                                 // グローバルの前計算などを初期状態に戻しておく
    rng = save;                                    // 乱数の状態を戻す
}

// =============================================================================
// 8. main
// =============================================================================
int main() {
    timer.start();      // ★ 一番最初にタイマー開始
    load_params();      // 環境変数からパラメータを読む (optuna 用)
    read_input();
    verify_diff();      // ★差分計算の自動検証 (提出時に不要なら VERIFY_MOVES = 0)
    init_log_table();   // 採用判定用テーブル

    static State state; // State が大きくても良いように static 領域へ
    multi_start_annealing(state, TIME_LIMIT_MS);

    output(state);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)state.score);
    fprintf(stderr, "iter = %lld, accept = %lld (%.2f%%), time = %.1f ms\n",
            g_iter, g_accept, 100.0 * (double)g_accept / (double)max(1LL, g_iter), (double)timer.ms());
    fprintf(stderr, "starts = %lld\n", g_round);
    // ★差分計算のバグ検出: 下の 2 つがずれていたら calc_score / apply_move が間違っている
    fprintf(stderr, "[check] diff-sum = %.3f, full = %.3f\n",
            (double)state.score, (double)full_score(state));
    return 0;
}
