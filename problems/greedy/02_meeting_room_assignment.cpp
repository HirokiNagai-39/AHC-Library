// =============================================================================
//  [貪欲系 02] 会議室割当 (interval scheduling with rooms)
// =============================================================================
//  【問題】
//    N 件の予約依頼がある。依頼 i は時刻 [s_i, e_i) の間ずっと 1 つの部屋を占有し、
//    受理すると価値 p_i が得られる。部屋は M 個あり、同じ部屋で時間が重なる依頼は
//    同時に受理できない。受理する依頼と割り当てる部屋を決め、価値の合計を最大化せよ。
//  【入力】
//    N M TMAX
//    s_i e_i p_i   (N 行、0 <= s_i < e_i <= TMAX の整数)
//  【出力】
//    N 行。依頼 i に割り当てた部屋番号 (0..M-1)。受理しないなら -1。
//  【スコア】 受理した依頼の価値の合計 (大きいほど良い)。重複割当があれば 0 点。
//  【入力生成方法】
//    N=300, M=6, TMAX=1000 固定。s_i は [0,TMAX) 一様、長さ len は [10,200] 一様
//    (e_i = min(s_i+len, TMAX))。p_i = round(len * u), u は [0.5,1.5] 一様。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     一見「依頼を部屋に割り当てる」問題だが、区間スケジューリングには次の性質がある。
//       どの時刻でも同時進行が M 件以下 ⇔ M 個の部屋に割り振れる
//     (区間グラフの彩色数は最大クリーク数に等しく、開始時刻順の貪欲彩色で最適解が得られる)。
//     つまり探索中は部屋を決める必要がなく、「時刻ごとの同時進行数」だけを見ればよい。
//     こうすると候補数が 1/M になり、部屋の決め打ちによる取りこぼしも無くなる。
//
//   ● 状態 (State)
//     各依頼を採用したかどうかと、時刻ごとの同時進行数 cnt[t]。
//     cnt[t] は 64bit のビット列を M 枚重ねた形で持ち、区間 [s,e) が入るかを
//     ビット演算でまとめて判定する (1 依頼あたり十数命令)。
//
//   ● 手の作り方
//     「まだ採用していない依頼のうち、そのまま入るもの」を候補にする。
//     入るものが無くなったら第 2 フェーズに移り、
//     「重なっている依頼を追い出してでも入れると価値の合計が増える依頼」を探して入れ替える。
//     最後に、採用した依頼を開始時刻順に並べて貪欲彩色し、部屋番号を割り当てる。
//
//   ● 評価値
//     p_i / (占有時間 + EPS_DIST)^ALPHA  … 「単位時間あたりの価値」。
//     長時間ふさぐ依頼は、価値が高くても他を何件も潰すので割に合わない、という考え方。
//     EPS_DIST は極端に短い依頼が評価値を独占するのを防ぐ下駄 (20 が最良だった)。
//
//   ● 差分計算 / 高速化
//     評価値は依頼ごとに定数なので入力読み込み時に前計算し、探索中は powf を呼ばない。
//     実行可能判定はビット演算なので O(区間長/64)。
//     ノイズ付きの貪欲を時間いっぱい繰り返して最良解を採用する。
//
//   ● つまずきポイント
//     ・最初は探索中から部屋を決めていた (依頼 × 部屋 が候補)。候補数が M 倍になるうえ、
//       たまたま埋まった部屋のせいで入れられる依頼を取りこぼしていた。
//     ・「入らなくなったら終わり」にすると、価値の低い依頼が場所を占領したままになる。
//       追い出し (第 2 フェーズ) を入れると素直に伸びる。
//
//   ● さらに伸ばすなら
//     ・追い出した依頼を別の場所に入れ直す再帰的な ejection chain
//     ・焼きなましに移して「採用集合」を直接動かす
//
//  【改善】
//    ・部屋を先に決めるのをやめ、cnt[t] <= M だけを見るようにした (部屋は最後に貪欲彩色で決める)。
//      部屋の決め打ちで入らなくなる取りこぼしが無くなり、候補数も 1/M になって試行数が増えた
//    ・入らなくなった後に「重なる依頼を追い出して入れる」改善フェーズを足した (価値が増えるときだけ)
//    ・評価値を前計算し powf を毎回呼ばないようにした。EPS_DIST 1 -> 20
//    seed 0..4 合計: 変更前 34027 -> 変更後 36438 (+7.1%)
//
//  【採用したライブラリ】 greedy/1_greedy
//    実測比較 (seed 0,1,2 の合計スコア): 1_greedy=20495 / 3_rolling_horizon=14807
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
float EPS_DIST  = 20.0f;   // p2: 長さの効かせ方をゆるめる下駄
float NOISE     = 0.30f;   // p3: ランダム貪欲のノイズ幅 (0 なら決定的な貪欲 1 回)
int   MAX_LS    = 200;     // p4: 追い出し挿入 (改善フェーズ) の最大手数

void load_params() {
    pick_env("p1", ALPHA);
    pick_env("p2", EPS_DIST);
    pick_env("p3", NOISE);
    pick_env("p4", MAX_LS);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (会議室割当)
// #############################################################################
constexpr int MAXN = 400, MAXM = 8, MAXT = 1024, TW = MAXT / 64;
constexpr int MAX_CAND = MAXN;             // ★候補は依頼だけ (部屋は最後にまとめて色分けする)

int N, M, TMAX, MAXLEN;
static int SS[MAXN], EE[MAXN], PP[MAXN];
static float DENS[MAXN];                   // 前計算した評価値 p / (長さ+EPS)^ALPHA (powf を毎回呼ばない)

// gain < 0 … 空いている所にそのまま入れる (構築フェーズ)
// gain > 0 … 重なる依頼を追い出して入れる (改善フェーズ。gain は価値の増分)
struct Move { int16_t req; float gain; };

//  ★「どの部屋か」は最後まで決めない。
//    区間スケジューリングは「どの時刻でも同時進行が M 件以下」なら必ず M 部屋に割り振れる
//    (区間グラフは開始時刻順の貪欲彩色で最適)。so 途中は cnt[t] だけ見れば良く、
//    部屋を先に決めてしまう貪欲より受理できる依頼が増える。
//    cnt[t] は「cnt[t] > j か」のビット列 lv[j] を M 枚持って表す。
struct State {
    float    score;
    uint64_t lv[MAXM][TW];
    int16_t  asg[MAXN];        // 出力用 (試行の最後に色分けして埋める)
    int16_t  acc[MAXN];        // 受理した依頼 (開始時刻順)
    int16_t  nacc;
    int16_t  nls;              // 改善フェーズで打った手数 (暴走よけ)
    uint8_t  tak[MAXN];        // 受理済みか
};

// [a,b) をワード単位のマスクにする (長さ 200 以下なので高々 4 ワード)
static inline void make_range(int a, int b, uint64_t *rem, int &w0, int &w1) {
    w0 = a >> 6; w1 = (b - 1) >> 6;
    for (int w = w0; w <= w1; w++) {
        const int lo = (w == w0) ? (a & 63) : 0;
        const int hi = (w == w1) ? ((b - 1) & 63) + 1 : 64;
        rem[w - w0] = (hi == 64 ? ~0ULL : ((1ULL << hi) - 1)) ^ ((1ULL << lo) - 1);
    }
}
// [a,b) がどこも満室でない (= もう 1 件入れられる) か
static inline bool can_take(const State &s, int a, int b) {
    uint64_t rem[8]; int w0, w1; make_range(a, b, rem, w0, w1);
    for (int w = w0; w <= w1; w++) if (s.lv[M - 1][w] & rem[w - w0]) return false;
    return true;
}
static inline void cnt_add(State &s, int a, int b) {          // cnt[t] += 1 (一番下の空き段を立てる)
    uint64_t rem[8]; int w0, w1; make_range(a, b, rem, w0, w1);
    for (int j = 0; j < M; j++) {
        bool any = false;
        for (int w = w0; w <= w1; w++) {
            const uint64_t add = rem[w - w0] & ~s.lv[j][w];
            s.lv[j][w] |= add; rem[w - w0] &= ~add;
            if (rem[w - w0]) any = true;
        }
        if (!any) break;
    }
}
static inline void cnt_sub(State &s, int a, int b) {          // cnt[t] -= 1 (一番上の段を落とす)
    uint64_t rem[8]; int w0, w1; make_range(a, b, rem, w0, w1);
    for (int j = M - 1; j >= 0; j--) {
        bool any = false;
        for (int w = w0; w <= w1; w++) {
            const uint64_t clr = rem[w - w0] & s.lv[j][w];
            s.lv[j][w] &= ~clr; rem[w - w0] &= ~clr;
            if (rem[w - w0]) any = true;
        }
        if (!any) break;
    }
}

// ---- 受理済みリスト (開始時刻順) ----
static inline int acc_lb(const State &s, int x) {
    int lo = 0, hi = s.nacc;
    while (lo < hi) { const int mid = (lo + hi) >> 1; if (SS[s.acc[mid]] >= x) hi = mid; else lo = mid + 1; }
    return lo;
}
static inline void acc_insert(State &s, int i) {
    const int p = acc_lb(s, SS[i]);
    memmove(&s.acc[p + 1], &s.acc[p], sizeof(int16_t) * (size_t)(s.nacc - p));
    s.acc[p] = (int16_t)i; s.nacc++;
}
static inline void acc_remove(State &s, int i) {
    for (int t = 0; t < s.nacc; t++) if (s.acc[t] == i) {
        memmove(&s.acc[t], &s.acc[t + 1], sizeof(int16_t) * (size_t)(s.nacc - t - 1)); s.nacc--; return;
    }
}

void init_state(State &s) {
    s.score = 0.0f; s.nacc = 0; s.nls = 0;
    memset(s.lv, 0, sizeof(uint64_t) * (size_t)M * TW);
    memset(s.tak, 0, sizeof(uint8_t) * (size_t)N);
}

// 開始時刻順の貪欲彩色。cnt[t] <= M なので必ず M 部屋に収まる
static void recolor(State &s) {
    int endt[MAXM];
    for (int k = 0; k < M; k++) endt[k] = -1;
    for (int i = 0; i < N; i++) s.asg[i] = -1;
    for (int t = 0; t < s.nacc; t++) {
        const int r = s.acc[t];
        int k = 0;
        while (k < M && endt[k] > SS[r]) k++;
        if (k >= M) k = 0;                                   // 起きないはず
        endt[k] = EE[r]; s.asg[r] = (int16_t)k;
    }
}

// 依頼 i を入れるために外す依頼の集合 (満室の時刻を左から順に、価値最小の依頼を外して潰す)
struct EPlan { int cnt; int16_t job[8]; float lost; };
static bool plan_eject(const State &s, int i, EPlan &pl) {
    const int a = SS[i], b = EE[i];
    uint64_t sat[8]; int w0, w1; make_range(a, b, sat, w0, w1);
    for (int w = w0; w <= w1; w++) sat[w - w0] &= s.lv[M - 1][w];
    pl.cnt = 0; pl.lost = 0.0f;
    while (true) {
        int t = -1;
        for (int w = w0; w <= w1; w++) if (sat[w - w0]) { t = (w << 6) + __builtin_ctzll(sat[w - w0]); break; }
        if (t < 0) return true;
        if (pl.cnt >= 8) return false;
        int bj = -1; float bp = 3.0e38f;
        for (int q = acc_lb(s, t - MAXLEN + 1); q < s.nacc; q++) {
            const int r = s.acc[q];
            if (SS[r] > t) break;
            if (EE[r] <= t) continue;
            bool already = false;
            for (int u = 0; u < pl.cnt; u++) if (pl.job[u] == r) { already = true; break; }
            if (already) continue;
            if ((float)PP[r] < bp) { bp = (float)PP[r]; bj = r; }
        }
        if (bj < 0) return false;
        pl.job[pl.cnt++] = (int16_t)bj; pl.lost += bp;
        uint64_t rm[8]; int u0, u1; make_range(SS[bj], EE[bj], rm, u0, u1);
        for (int w = max(w0, u0); w <= min(w1, u1); w++) sat[w - w0] &= ~rm[w - u0];
    }
}

// そのまま入る依頼を候補にする。1 つも入らなくなったら「追い出しても得になる挿入」を候補にする
inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    for (int i = 0; i < N; i++) if (!s.tak[i] && can_take(s, SS[i], EE[i])) {
        out[m].req = (int16_t)i; out[m].gain = -1.0f; m++;
    }
    if (m > 0) return m;
    if (s.nls < MAX_LS) {
        EPlan pl;
        for (int i = 0; i < N; i++) if (!s.tak[i]) {
            if (!plan_eject(s, i, pl)) continue;
            const float g = (float)PP[i] - pl.lost;
            if (g > 0.5f) { out[m].req = (int16_t)i; out[m].gain = g; m++; }
        }
        if (m > 0) return m;
    }
    recolor(const_cast<State &>(s));                          // 試行の最後に部屋へ色分けする
    return 0;
}
// 評価値: 構築フェーズは単位時間あたりの価値 (前計算済み)、改善フェーズは価値の増分
inline float eval_move(const State &s, const Move &mv) { (void)s; return mv.gain < 0.0f ? DENS[mv.req] : mv.gain; }
inline float calc_score(const State &s, const Move &mv) { (void)s; return mv.gain < 0.0f ? (float)PP[mv.req] : mv.gain; }
inline void apply_move(State &s, const Move &mv) {
    const int i = mv.req;
    if (mv.gain >= 0.0f) {
        EPlan pl; plan_eject(s, i, pl);                       // 走査時と同じ計画 (状態は変わっていない)
        for (int t = 0; t < pl.cnt; t++) {
            const int r = pl.job[t];
            cnt_sub(s, SS[r], EE[r]); s.tak[r] = 0; acc_remove(s, r);
        }
        s.nls++;
    }
    cnt_add(s, SS[i], EE[i]); s.tak[i] = 1; acc_insert(s, i);
}

void read_input() {
    if (scanf("%d %d %d", &N, &M, &TMAX) == 3 && N >= 1) {
        N = min(N, MAXN); M = min(M, MAXM); TMAX = min(TMAX, MAXT);
        for (int i = 0; i < N; i++) scanf("%d %d %d", &SS[i], &EE[i], &PP[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 22);
        N = 300; M = 6; TMAX = 1000;
        for (int i = 0; i < N; i++) {
            SS[i] = (int)g.next((uint32_t)TMAX);
            const int len = 10 + (int)g.next(191);
            EE[i] = min(SS[i] + len, TMAX);
            PP[i] = (int)((float)(EE[i] - SS[i]) * (0.5f + g.nextf()));
            if (PP[i] < 1) PP[i] = 1;
        }
    }
    MAXLEN = 1;
    for (int i = 0; i < N; i++) {
        DENS[i] = (float)PP[i] / powf((float)(EE[i] - SS[i]) + EPS_DIST, ALPHA);
        MAXLEN = max(MAXLEN, EE[i] - SS[i]);
    }
}

void output(const State &s) {
    string r; r.reserve((size_t)N * 3);
    for (int i = 0; i < N; i++) { r += to_string((int)s.asg[i]); r += '\n'; }
    fputs(r.c_str(), stdout);
}

// [a,b) のビットが全部空いているか
static inline bool range_free(const uint64_t *o, int a, int b) {
    while (a < b) {
        const int w = a >> 6, lo = a & 63;
        const int hi = min(64, lo + (b - a));
        const uint64_t mask = (hi == 64 ? ~0ULL : ((1ULL << hi) - 1)) ^ ((1ULL << lo) - 1);
        if (o[w] & mask) return false;
        a += hi - lo;
    }
    return true;
}
static inline void range_set(uint64_t *o, int a, int b) {
    while (a < b) {
        const int w = a >> 6, lo = a & 63;
        const int hi = min(64, lo + (b - a));
        const uint64_t mask = (hi == 64 ? ~0ULL : ((1ULL << hi) - 1)) ^ ((1ULL << lo) - 1);
        o[w] |= mask;
        a += hi - lo;
    }
}

float replay_true_score(const State &s) {
    static uint64_t occ[MAXM][TW];
    memset(occ, 0, sizeof(occ));
    float total = 0.0f;
    for (int i = 0; i < N; i++) {
        const int k = s.asg[i];
        if (k < 0) continue;
        if (k >= M || !range_free(occ[k], SS[i], EE[i])) { fprintf(stderr, "[error] 依頼 %d が重複\n", i); return -1.0f; }
        range_set(occ[k], SS[i], EE[i]);
        total += (float)PP[i];
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
