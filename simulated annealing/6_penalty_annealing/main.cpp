// =============================================================================
//  ペナルティ付き焼きなまし (Penalty Annealing)   ---  AHC 用 高速テンプレート
// =============================================================================
//  制約付きの問題で「制約を破った解 (invalid) も探索空間に含めて」焼きなます。
//    評価値 = 素点 (raw) + W(t) × 制約違反量 (viol)
//  ペナルティ係数 W(t) は時間経過で指数的に増加させる:
//    序盤: W が小さい → 制約を破って自由に探索できる (valid 解だけでは近傍が繋がらない問題に有効)
//    終盤: W が巨大   → 違反が 1 でも残ると大損なので、自然と valid 解へ収束する
//
//  【最終的に valid な解になる保険が 2 段ある】
//    (1) 探索中、valid な解の中での最良解 (best) を別に保持し、最後に採用する
//    (2) それでも violation が残った場合、最後の FORCE_VALID_MS だけ
//        「違反量が減る手だけを受け入れる山登り」で強制的に修復する
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct State { float raw, viol; ... };  ... 状態 (raw = 素点, viol = 制約違反量 >= 0)
//    init_state(State&)   ... 初期解構築 (raw と viol も必ずセットする。invalid でも良い)
//    modify(State&)       ... 遷移(近傍)を 1 つ選ぶ ★この時点では state を変えない
//    calc_score(State&)   ... 選んだ遷移の「素点差分 g_dr」「違反量差分 g_dv」をセットする
//    apply_move(State&)   ... 採用が決まった遷移を実際に state へ反映する
//
//  【違反量 viol の設計指針】
//    ・「違反の量」を連続的に測る (例: 容量超過分、長さ超過分)。0/1 の個数だけより
//      「どちらの invalid がよりマシか」の勾配が付き、修復方向へ誘導しやすい
//    ・viol は素点と同じくらいのスケールに正規化しておくと W の調整が楽
//    ・W の終値 PEN_END は「素点で得られる最大の得よりずっと大きく」する (violation 1 単位を
//      残す得が絶対に無いように)。これが「最終的に valid になる」ための条件
//
//  (デモ: 辺長制約付き TSP。総移動距離を最小化、ただし全ての辺は LMAX 以下でなければならない。
//   viol = Σ max(0, 辺長 - LMAX)。初期解はランダムなので violation だらけから始まる)
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
// ★温度は「素点の差分」のスケールに合わせる (通常の焼きなましと同じ)。
// ★ペナルティ係数 W は指数スケジュールで PEN_START → PEN_END と増える。
//    PEN_START ≒ 「違反 1 単位と素点 1 単位が同じくらいの重さ」になる値 (viol を素点と同じ
//                 スケールにしてあれば 1 前後から)
//    PEN_END   ≒ 素点で得られる最大の得よりずっと大きい値 (violation を残す得が無いように)
float TEMP_START = 50.0f;    // p1: 開始温度
float TEMP_END   = 1.0f;     // p2: 終了温度
float PEN_START  = 1.0f;     // p3: ペナルティ係数の初期値
float PEN_END    = 1000.0f;  // p4: ペナルティ係数の最終値 (★大きくして valid へ収束させる)
float P_ORO      = 0.30f;    // p5: 近傍の選び方 (デモ: or-opt を選ぶ確率)

void load_params() {
    pick_env("p1", TEMP_START);
    pick_env("p2", TEMP_END);
    pick_env("p3", PEN_START);
    pick_env("p4", PEN_END);
    pick_env("p5", P_ORO);
}

// =============================================================================
// 4. 素点の最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
// ※ 切り替えの対象は「素点 raw」のみ。違反量 viol は常に最小化 (0 が valid) する。
// constexpr bool MAXIMIZE = true;     // ← 素点最大化のとき こちらを有効化
constexpr bool MAXIMIZE = false;       // ← 素点最小化のとき こちらを有効化 (デモの TSP は最小化)

// gain (改善量) = GAIN_SIGN * 素点差分。 gain > 0 なら「良くなった」
constexpr float GAIN_SIGN = MAXIMIZE ? 1.0f : -1.0f;
// a が b より良ければ true (素点の比較)
inline bool is_better(float a, float b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS   = 1900.0f;  // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr int   ITER_PER_CHECK  = 1024;     // ★ 時間計測はこの回数に 1 回だけ行う
constexpr bool  ADAPTIVE_BLOCK  = true;     // 1 ブロックの実行時間が一定になるよう回数を自動調整
constexpr float TARGET_BLOCK_MS = 0.5f;     // ADAPTIVE_BLOCK=true のときの 1 ブロックの目標時間[ms]

// ---- ペナルティ付き焼きなまし用 ----
constexpr bool  KEEP_BEST      = true;    // valid な最良解を別に保持する (State のコピーが重いなら false)
constexpr float VALID_EPS      = 1e-3f;   // viol がこれ以下なら valid とみなす (float 誤差の吸収)
constexpr float FORCE_VALID_MS = 100.0f;  // 終了間際、強制修復フェーズに使う時間[ms] (保険その2)
constexpr bool  RESYNC_VIOL    = true;    // ブロックごとに viol を全計算し直して float 誤差を消す
                                          // (full_viol が重い問題では false にする)

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


// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■
// #     (デモ: 辺長制約付き TSP / 総移動距離の最小化、ただし全ての辺は LMAX 以下)
// #     標準入力が無ければランダムに問題を生成するので、そのまま実行できる。
// #############################################################################
constexpr int   MAXN = 1000;            // 問題サイズの上限
constexpr float LMAX = 200.0f;          // ★制約 (デモ): 使って良い辺の長さの上限
int   N;                                // 都市数
static float DIST[MAXN * MAXN];         // 距離行列 (i*N+j でアクセス。stride を N にして cache に優しく)
static inline float dist(int i, int j) { return DIST[i * N + j]; }
// 辺 (i,j) の違反量 = 長さの超過分 (使う辺が LMAX 以下なら 0)
static inline float pend(int i, int j) { const float d = dist(i, j) - LMAX; return d > 0.0f ? d : 0.0f; }

// ★必須: 状態。float raw (素点) と float viol (制約違反量 >= 0) を必ず持たせる。
struct State {
    float raw;          // ★必須: 素点 (制約を無視したスコア)
    float viol;         // ★必須: 制約違反量 (0 なら valid)
    int   ord[MAXN];    // 以下は問題ごとに書き換える (ord[i] = i 番目に訪れる都市)
};

// modify() が選んだ遷移を保持しておく置き場 (差分計算 / 適用 で使う)
struct Move { int type, i, j; };
static Move g_mv;

// calc_score() の結果の置き場 (g_dr = 素点差分, g_dv = 違反量差分)
static float g_dr, g_dv;

// 素点の全計算 (初期解のスコア設定・デバッグ用)。差分計算のバグ検出に使える。
float full_raw(const State &s) {
    float t = 0.0f;
    for (int i = 0; i < N; i++) t += dist(s.ord[i], s.ord[i + 1 == N ? 0 : i + 1]);
    return t;
}
// 違反量の全計算
float full_viol(const State &s) {
    float t = 0.0f;
    for (int i = 0; i < N; i++) t += pend(s.ord[i], s.ord[i + 1 == N ? 0 : i + 1]);
    return t;
}

// ★必須: 初期解構築 (raw と viol も必ずセットする。invalid な初期解でも良いのがこの手法の強み)
void init_state(State &s) {
    for (int i = 0; i < N; i++) s.ord[i] = i;
    rnd_shuffle(s.ord, N);          // デモではランダム初期解 = violation だらけから始める
    s.raw  = full_raw(s);           // ★ raw を必ず設定
    s.viol = full_viol(s);          // ★ viol を必ず設定
}

// ★必須: 遷移(近傍)を 1 つランダムに選ぶ。★ここでは state を変更しない (採用時のみ適用する)
inline void modify(State &s) {
    if (rng.nextf() < P_ORO) {
        // or-opt : 位置 i の都市を 位置 j の直後へ移動
        g_mv.type = 1;
        int i = rng.next(N), j;
        do { j = rng.next(N); } while (j == i || j == (i == 0 ? N - 1 : i - 1));
        g_mv.i = i; g_mv.j = j;
    } else {
        // 2-opt : 区間 [i, j] を反転
        g_mv.type = 0;
        int i = 1 + rng.next(N - 2);
        int j = i + 1 + rng.next(N - 1 - i);
        g_mv.i = i; g_mv.j = j;
    }
}

// ★必須: 選んだ遷移の「素点差分 g_dr」と「違反量差分 g_dv」をセットする (O(1) の差分計算が最重要)
//   素点と違反量は「消える辺 / 増える辺」が同じなので、まとめて計算すると速い。
inline void calc_score(State &s) {
    const int *p = s.ord;
    const int i = g_mv.i, j = g_mv.j;
    if (g_mv.type == 0) {
        // 2-opt: 辺 (p[i-1],p[i]) と (p[j],p[j+1]) が (p[i-1],p[j]) と (p[i],p[j+1]) に変わる
        const int a = p[i - 1], b = p[i], c = p[j], d = p[j + 1 == N ? 0 : j + 1];
        g_dr = (dist(a, c) + dist(b, d)) - (dist(a, b) + dist(c, d));
        g_dv = (pend(a, c) + pend(b, d)) - (pend(a, b) + pend(c, d));
    } else {
        // or-opt: 都市 c を抜いて (a,b) の間に挿入する
        const int c  = p[i];
        const int pi = p[i == 0 ? N - 1 : i - 1], ni = p[i + 1 == N ? 0 : i + 1];
        const int a  = p[j], b = p[j + 1 == N ? 0 : j + 1];
        g_dr = (dist(pi, ni) - dist(pi, c) - dist(c, ni))
             + (dist(a, c) + dist(c, b) - dist(a, b));
        g_dv = (pend(pi, ni) - pend(pi, c) - pend(c, ni))
             + (pend(a, c) + pend(c, b) - pend(a, b));
    }
}

// 採用が決まった遷移を実際に state へ反映する
inline void apply_move(State &s) {
    const int i = g_mv.i, j = g_mv.j;
    if (g_mv.type == 0) {
        reverse(s.ord + i, s.ord + j + 1);
    } else {
        const int c = s.ord[i];
        if (j > i) { memmove(s.ord + i, s.ord + i + 1, (size_t)(j - i) * sizeof(int));         s.ord[j] = c; }
        else       { memmove(s.ord + j + 2, s.ord + j + 1, (size_t)(i - j - 1) * sizeof(int)); s.ord[j + 1] = c; }
    }
}

// 入力 (デモ: 標準入力が無ければランダムに問題を生成)
void read_input() {
    static float px[MAXN], py[MAXN];
    if (scanf("%d", &N) == 1 && N >= 4) {
        if (N > MAXN) N = MAXN;
        for (int i = 0; i < N; i++) scanf("%f %f", &px[i], &py[i]);
    } else {
        N = 200;
        Xor128 g; g.seed(20260824);
        for (int i = 0; i < N; i++) { px[i] = g.nextf() * 1000.0f; py[i] = g.nextf() * 1000.0f; }
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            const float dx = px[i] - px[j], dy = py[i] - py[j];
            DIST[i * N + j] = sqrtf(dx * dx + dy * dy);
        }
}

// 出力
void output(const State &s) {
    for (int i = 0; i < N; i++) printf("%d\n", s.ord[i]);
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
// 7. ペナルティ付き焼きなまし 本体
// =============================================================================
//  s を初期解として受け取り、deadline_ms まで焼きなます。
//  採用判定に使う改善量は  gain = GAIN_SIGN * (素点差分) - W * (違反量差分)
//  で、W (ペナルティ係数) は w0 → w1 へ指数的に増加させる。
//  探索中に見つけた「valid な解の中での最良解」は best に保持し、最後に s へ書き戻す。
//  t0, t1 : 開始 / 終了温度 (素点差分のスケールで決める。通常の焼きなましと同じ)
//  w0, w1 : ペナルティ係数の初期値 / 最終値 (w1 は素点の得よりずっと大きく)
static State g_best;                     // valid な解の中での最良解 (State が大きくても良いように static)
static bool  g_has_best = false;
static ll    g_best_update = 0;          // 統計: valid 最良解の更新回数

void anneal(State &s, float deadline_ms, float t0, float t1, float w0, float w1) {
    if constexpr (KEEP_BEST) {
        if (s.viol <= VALID_EPS) { g_best = s; g_has_best = true; }
    }

    const float begin = timer.ms();
    const float span  = deadline_ms - begin;
    if (span <= 0.0f) return;
    if (t0 < 1e-9f) t0 = 1e-9f;              // 0 除算 / NaN 対策
    if (t1 < 1e-9f) t1 = 1e-9f;
    if (t1 > t0)    t1 = t0;                 // 念のため t0 >= t1 にしておく
    if (w0 < 1e-9f) w0 = 1e-9f;
    if (w1 < w0)    w1 = w0;                 // W は増加スケジュールにしておく
    const float lr = logf(t1 / t0);          // 温度: 指数スケジュール用
    const float lw = logf(w1 / w0);          // ペナルティ係数: 指数スケジュール用

    float temp    = t0;
    float pen     = w0;
    int   block   = ITER_PER_CHECK;          // ★ この回数に 1 回だけ時間計測する
    float prev    = begin;

    while (true) {
        // ---- 内側ループ: 時間計測も温度・係数の更新も一切しない (ここが最速であるべき) ----
        for (int it = 0; it < block; it++) {
            modify(s);                                  // 遷移を 1 つ選ぶ
            calc_score(s);                              // g_dr (素点差分), g_dv (違反量差分)
            const float gain = GAIN_SIGN * g_dr - pen * g_dv;   // ペナルティ込みの改善量
            // gain > T*log(u) : 改善なら必ず採用、悪化なら exp(gain/T) の確率で採用
            if (gain > temp * g_log_tbl[rng.next() & LOG_TBL_MASK]) {
                apply_move(s);                          // 採用時だけ実際に反映
                s.raw  += g_dr;
                s.viol += g_dv;
                if (s.viol < 0.0f) s.viol = 0.0f;       // float 誤差で負にならないように
                g_accept++;
                if constexpr (KEEP_BEST) {              // valid な解だけを best の候補にする
                    if (s.viol <= VALID_EPS && (!g_has_best || is_better(s.raw, g_best.raw))) {
                        g_best = s; g_has_best = true; g_best_update++;
                    }
                }
            }
            // else rollback(s);   // ←「先に適用して却下時に戻す」方式にする場合はここ
        }
        g_iter += block;

        // ---- ここからブロックごとの処理 (時間計測・温度とペナルティ係数の更新) ----
        const float now = timer.ms();
        if (now >= deadline_ms) break;
        const float r = (now - begin) / span;           // 進捗 [0,1)
        temp = t0 * expf(lr * r);                       // 温度: 指数で下げる
        pen  = w0 * expf(lw * r);                       // ★ペナルティ係数: 指数で上げる
        if constexpr (RESYNC_VIOL) s.viol = full_viol(s);   // float 誤差の蓄積を消す (O(全計算) 1 回/ブロック)

        tune_block(block, now - prev); prev = now;      // 1 ブロックの実行時間を一定に保つ
    }

    // valid な最良解が見つかっていれば書き戻す (現在解が invalid、または素点で負けている場合)
    if constexpr (KEEP_BEST) {
        if (g_has_best && (s.viol > VALID_EPS || is_better(g_best.raw, s.raw))) s = g_best;
    }
}

// =============================================================================
//  強制修復フェーズ (保険その2)
// =============================================================================
//  ペナルティを増やしても violation が残った場合、残り時間で
//  「違反量が減る手 (または違反量そのままで素点が良くなる手) だけを受け入れる山登り」を行う。
//  valid になった時点で即終了する。
void force_valid(State &s, float deadline_ms) {
    constexpr float DV_EPS = 1e-6f;
    while (s.viol > VALID_EPS && timer.ms() < deadline_ms) {
        for (int it = 0; it < 256; it++) {
            modify(s);
            calc_score(s);
            const bool ok = (g_dv < -DV_EPS) ||                                  // 違反量が減る
                            (g_dv <= DV_EPS && GAIN_SIGN * g_dr > 0.0f);         // 違反量そのまま素点改善
            if (ok) {
                apply_move(s);
                s.raw  += g_dr;
                s.viol += g_dv;
                if (s.viol < 0.0f) s.viol = 0.0f;
            }
        }
        s.viol = full_viol(s);          // 判定に使うので毎回同期しておく (O(全計算) 1 回/256 手)
    }
}

// =============================================================================
//  ★差分計算の自動検証 (main の最初に走る)
// =============================================================================
//  1 手ずつ「calc_score() がセットした g_dr / g_dv」対「実際に apply_move して
//  full_raw() / full_viol() を取り直した変化量」を突き合わせる。
//  採用・却下に関係なく全部の手を検査するので、
//    ・却下された手の calc_score が間違っている
//    ・apply_move が raw / viol を上書きしていて差分の誤りが打ち消されている
//  といった「探索が静かに弱くなるだけで最後の値は合う」バグも捕まえられる。
//  ※ 乱数の状態は前後で復元するので、VERIFY_MOVES を変えても探索結果は 1 ビットも変わらない。
//  ※ 提出時に消したければ VERIFY_MOVES = 0 にする (数 ms しか掛からないので普段は付けたままで良い)。
constexpr int VERIFY_MOVES = 200;

static void verify_diff() {
    if constexpr (VERIFY_MOVES <= 0) return;
    const Xor128 save = rng;                       // 乱数列を汚さないよう退避
    static State s;
    init_state(s);
    int bad_r = 0, bad_v = 0;
    for (int i = 0; i < VERIFY_MOVES; i++) {
        const float br = full_raw(s), bv = full_viol(s);
        modify(s);
        calc_score(s);
        const float dr = g_dr, dv = g_dv;
        apply_move(s);                             // ★必ず適用して先へ進む (ランダムウォークで色々な状態を通す)
        s.raw += dr; s.viol += dv; if (s.viol < 0.0f) s.viol = 0.0f;   // フレームワークと同じ更新をする
        const float ar = full_raw(s), av = full_viol(s);
        const float act_r = ar - br, act_v = av - bv;    // 実際に起きた変化
        // 許容誤差は「全計算の float 丸め (合計値に比例)」+「差分そのものの 0.1%」。
        const float tol_r = 1e-4f * max(1.0f, max(fabsf(br), fabsf(ar)))
                          + 1e-3f * max(fabsf(dr), fabsf(act_r));
        const float tol_v = 1e-4f * max(1.0f, max(fabsf(bv), fabsf(av)))
                          + 1e-3f * max(fabsf(dv), fabsf(act_v));
        if (fabsf(act_r - dr) > tol_r) {
            if (++bad_r <= 5)
                fprintf(stderr, "[verify] NG %d 手目 (素点): g_dr=%.4f / 実際の変化=%.4f (ずれ %.4f)\n",
                        i, (double)dr, (double)act_r, (double)(act_r - dr));
        }
        if (fabsf(act_v - dv) > tol_v) {
            if (++bad_v <= 5)
                fprintf(stderr, "[verify] NG %d 手目 (違反量): g_dv=%.4f / 実際の変化=%.4f (ずれ %.4f)\n",
                        i, (double)dv, (double)act_v, (double)(act_v - dv));
        }
    }
    if (bad_r) fprintf(stderr, "[verify] ★素点の差分が %d/%d 手でずれています。calc_score か apply_move にバグがあります\n", bad_r, VERIFY_MOVES);
    if (bad_v) fprintf(stderr, "[verify] ★違反量の差分が %d/%d 手でずれています。calc_score か apply_move にバグがあります\n", bad_v, VERIFY_MOVES);
    if (!bad_r && !bad_v)
        fprintf(stderr, "[verify] 差分計算 OK (素点・違反量とも %d 手を全計算と突き合わせ)\n", VERIFY_MOVES);
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
    init_state(state);  // ★ 初期解構築 (invalid でも良い)
    const float init_viol = state.viol;

    // 本体: 強制修復フェーズの分だけ手前で切り上げる
    anneal(state, TIME_LIMIT_MS - FORCE_VALID_MS, TEMP_START, TEMP_END, PEN_START, PEN_END);

    // 保険その2: それでも violation が残っていれば残り時間で強制修復
    if (state.viol > VALID_EPS) force_valid(state, TIME_LIMIT_MS);

    output(state);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    const bool valid = state.viol <= VALID_EPS;
    if (!valid) fprintf(stderr, "[warn] ★invalid なまま終了しました (viol=%.3f)。PEN_END を上げる / 制約の可否を確認\n", (double)state.viol);
    fprintf(stderr, "Score = %.0f\n", (double)state.raw);
    fprintf(stderr, "viol  = %.3f -> %.3f (%s), best_update = %lld\n",
            (double)init_viol, (double)state.viol, valid ? "valid" : "INVALID", g_best_update);
    fprintf(stderr, "iter = %lld, accept = %lld (%.2f%%), time = %.1f ms\n",
            g_iter, g_accept, 100.0 * (double)g_accept / (double)max(1LL, g_iter), (double)timer.ms());
    // ★差分計算のバグ検出: 下の 2 つがずれていたら calc_score / apply_move が間違っている
    fprintf(stderr, "[check] raw: diff-sum = %.3f, full = %.3f / viol: diff-sum = %.3f, full = %.3f\n",
            (double)state.raw, (double)full_raw(state), (double)state.viol, (double)full_viol(state));
    return 0;
}
