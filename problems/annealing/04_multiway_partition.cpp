// =============================================================================
//  [焼きなまし系 04] 多群数分割 (グループ和の均等化)
// =============================================================================
//  【問題】
//    N 個の正整数 a_0..a_{N-1} を K 個のグループに分ける (空グループがあっても良い)。
//    グループ k に入れた数の合計を S_k とするとき、max_k S_k - min_k S_k を最小化せよ。
//  【入力】
//    N K
//    a_0 ... a_{N-1}
//  【出力】
//    N 行。数 i を入れたグループ番号 (0..K-1)。
//  【スコア】 max_k S_k - min_k S_k (小さいほど良い、0 が完全な均等分割)。
//  【入力生成方法】
//    N=300, K=10 固定。a_i は 1..1000000 の一様整数。
//    値の桁が大きいので貪欲では揃わず、細かい入れ替えの積み重ねが必要になる。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     この問題は近傍設計がすべて。評価値が max-min という「一番悪い所だけを見る」形なので、
//     ランダムな 2 つを交換してもほとんどの手は max にも min にも触れず、完全な無駄になる。
//     そこで近傍を次の 1 種類に絞る。
//       「和が最大のグループ ga の数 i」と「和が最小のグループ gb の数 j」を入れ替える
//     さらに、両群の和は d = a_i - a_j だけ動くので、d = (S_ga - S_gb)/2 ならちょうど揃う。
//     つまり「値が a_i - (S_ga - S_gb)/2 に一番近い j」を選べば良い。
//     この狙い撃ちを入れる前後でスコアが 798 -> 5 (-99.4%) と桁違いに変わった。
//
//   ● 状態 (State)
//     数 -> グループの割当と、各グループの和 sum[]。
//     加えて、各グループの所属リストを「値の昇順」で保持する (二分探索のため)。
//
//   ● 手の作り方
//     最大和グループから数 i をランダムに選び、最小和グループの所属リストから
//     「値が a_i - d に一番近い j」を二分探索 O(log) で取り出して入れ替える。
//     狙い撃ちの確率を 0.3 -> 0.6 -> 0.95 -> 1.0 と上げるほど良くなり、最終的に 1.0 (常に狙い撃ち) にした。
//
//   ● 評価値
//     max_k S_k - min_k S_k。最小化。
//
//   ● 差分計算 / 高速化
//     1 手で動くのは 2 グループだけなので、
//     「和が大きい方の上位 2 グループ」と「小さい方の上位 2 グループ」を覚えておけば、
//     入れ替え後の max/min が O(1) で分かる (動いた 2 つを除いた残りの max/min が上位 2 つから分かる)。
//     所属リストは値の昇順を保つ必要があるので、挿入位置を二分探索して詰める。
//
//   ● つまずきポイント
//     ・max/min の更新を素直に O(K) で回すと、狙い撃ち近傍の速さが活きない。
//     ・所属リストの昇順を保つ更新を忘れると二分探索が壊れる。スコアには即座に出ないので危険
//       ([verify] は差分計算しか見ないため、ここは A/B 比較で確認するしかない)。
//
//   ● さらに伸ばすなら
//     ・すでに理論下限に到達している。総和が K で割り切れる seed では 0、
//       割り切れない seed では max-min >= 1 なので 1 が下限で、seed 10-29 まで広げても全部下限に届いた。
//     ・伸ばすなら N や K を大きくして、Karmarkar-Karp 法などの構築的な初期解を試す方向
//
//  【改善】
//    (1) 近傍を「最大和グループ <-> 最小和グループ の交換」だけに絞り、相手は
//        「和がちょうど揃う値に一番近いもの」を二分探索で選ぶ (所属リストを値順に保持)。
//    (2) max/min の再計算を上位 2 グループのキャッシュで O(K) -> O(1) に。
//    (3) 焼きなましの繰り返し (isa) は 1 回で十分だったので単純な焼きなまし (sa) へ載せ替え、
//        終了温度を 10 -> 40 に上げた。
//    seed 0..4 合計: 変更前 798 -> 変更後 5 (-99.4%)   ※seed 0..9 でも 10 (= 各 seed 1)
//    (総和が 10 で割り切れない seed では max-min >= 1 なので、これは理論下限に到達している)
//
//  【採用したライブラリ】 simulated annealing/3_simulated_annealing
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=859 / 2_hill_climbing_kick=1126 / 3_simulated_annealing=486 / 4_multi_start_annealing=470 / 5_iterated_annealing=404
// =============================================================================
// =============================================================================
//  焼きなまし法 (Simulated Annealing)   ---  AHC 用 高速テンプレート
// =============================================================================
//  確率的に悪化も受け入れて局所解を脱出する。AHC で最もよく使われる手法。
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
float TEMP_START = 100000.0f;      // p1: 開始温度
float TEMP_END   = 40.0f;          // p2: 終了温度
float P_MOVE     = 0.0f;           // p3: 「1 つの数を別グループへ移す」近傍を選ぶ確率
float P_TGT      = 1.0f;           // p4: 「最大和グループ <-> 最小和グループ の交換」を選ぶ確率
int   JITTER     = 0;              // p5: 選んだ相手を値の順で ±JITTER だけランダムにずらす

void load_params() {
    pick_env("p1", TEMP_START);
    pick_env("p2", TEMP_END);
    pick_env("p3", P_MOVE);
    pick_env("p4", P_TGT);
    pick_env("p5", JITTER);
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


// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (多群数分割)
// #############################################################################
constexpr int MAXN = 512, MAXK = 32;

int N, K;
static long long AV[MAXN];

// ---- グループごとの所属リスト (「最大和グループの要素」を O(1) で選ぶため) ----
static int       MEM[MAXK * MAXN];   // MEM[k*MAXN + t] : グループ k の t 番目の要素 (値の昇順)
static long long MEMV[MAXK * MAXN];  // その要素の値 (二分探索を 1 回のロードで回すため)
static int MCNT[MAXK];          // グループ k の要素数
static int MPOS[MAXN];          // 要素 i がリストの何番目にいるか
// ---- 和の大きい順 / 小さい順 上位 3 グループ ----
// 1 手で変わるのは高々 2 グループなので、上位 3 つを持っておけば
// 「その 2 つを除いた max / min」が O(1) で分かる (K 個を走査しなくて済む)。
static int TOP0, TOP1, BOT0, BOT1;
static uint32_t TH_MOVE, TH_TGT;

struct State {
    float     score;
    int8_t    grp[MAXN];
    long long sum[MAXK];
};

struct Move { int i, j, to; };     // j >= 0 なら i と j を入れ替え、そうでなければ i を to へ移す
static Move g_mv;

static inline float spread(const long long *sum) {
    long long mx = sum[0], mn = sum[0];
    for (int k = 1; k < K; k++) { if (sum[k] > mx) mx = sum[k]; if (sum[k] < mn) mn = sum[k]; }
    return (float)(mx - mn);
}

float full_score(const State &s) {
    static long long sum[MAXK];
    for (int k = 0; k < K; k++) sum[k] = 0;
    for (int i = 0; i < N; i++) sum[s.grp[i]] += AV[i];
    return spread(sum);
}

// グループ k のリストから位置 pos の要素を取り除く (値の昇順を保つ)
static inline void grp_erase(int k, int pos) {
    const int c = --MCNT[k];
    int *M = MEM + k * MAXN; long long *V = MEMV + k * MAXN;
    for (int t = pos; t < c; t++) { M[t] = M[t + 1]; V[t] = V[t + 1]; MPOS[M[t]] = t; }
}
// グループ k のリストへ要素 e を入れる (値の昇順を保つ)
static inline void grp_insert(int k, int e) {
    int t = MCNT[k]++;
    int *M = MEM + k * MAXN; long long *V = MEMV + k * MAXN;
    const long long v = AV[e];
    while (t > 0 && V[t - 1] > v) { M[t] = M[t - 1]; V[t] = V[t - 1]; MPOS[M[t]] = t; t--; }
    M[t] = e; V[t] = v; MPOS[e] = t;
}

static inline void rebuild_rank(const State &s) {
    int t0 = 0, t1 = -1, b0 = 0, b1 = -1;
    for (int k = 1; k < K; k++) {
        const long long v = s.sum[k];
        if (v > s.sum[t0])            { t1 = t0; t0 = k; }
        else if (t1 < 0 || v > s.sum[t1]) t1 = k;
        if (v < s.sum[b0])            { b1 = b0; b0 = k; }
        else if (b1 < 0 || v < s.sum[b1]) b1 = k;
    }
    TOP0 = t0; TOP1 = t1 < 0 ? t0 : t1;
    BOT0 = b0; BOT1 = b1 < 0 ? b0 : b1;
}

void init_state(State &s) {
    for (int k = 0; k < K; k++) { s.sum[k] = 0; MCNT[k] = 0; }
    for (int i = 0; i < N; i++) {
        const int k = (int)rng.next((uint32_t)K);
        s.grp[i] = (int8_t)k; s.sum[k] += AV[i];
        grp_insert(k, i);
    }
    rebuild_rank(s);
    TH_MOVE = (uint32_t)(P_MOVE * 4096.0f);
    TH_TGT  = (uint32_t)(P_TGT  * 4096.0f);
    s.score = spread(s.sum);
}

inline void modify(State &s) {
    const uint32_t r = rng.next();
    const uint32_t typ = r & 4095u;
    if (typ < TH_TGT) {
        // ---- 狙い撃ち近傍 ----
        // 最大和グループ ga と最小和グループ gb の間で 1 個ずつ交換する。和は d = a_i - a_j だけ動くので
        // d = (S_ga - S_gb)/2 なら 2 群がちょうど揃う。ga から適当に i を選び、
        // 「値が a_i - d に一番近い」相手 j を gb のソート済みリストから二分探索で選ぶ。
        const int ga = TOP0, gb = BOT0;
        const int ca = MCNT[ga], cb = MCNT[gb];
        if (ga != gb && ca > 0 && cb > 0) {
            const long long tg = (s.sum[ga] - s.sum[gb]) >> 1;
            const int i = MEM[ga * MAXN + (int)rng.next((uint32_t)ca)];
            const long long want = AV[i] - tg;         // 値がこれに一番近い j を選びたい
            const long long *V = MEMV + gb * MAXN;
            int lo = 0, hi = cb;                       // V は昇順なので二分探索
            while (lo < hi) { const int m = (lo + hi) >> 1; if (V[m] < want) lo = m + 1; else hi = m; }
            if (lo > 0 && (lo >= cb || want - V[lo - 1] <= V[lo] - want)) lo--;
            if (JITTER) {                              // 多様性のため近い順から少しずらす
                lo += (int)rng.next((uint32_t)(2 * JITTER + 1)) - JITTER;
                if (lo < 0) lo = 0; else if (lo >= cb) lo = cb - 1;
            }
            g_mv.i  = i;
            g_mv.j  = MEM[gb * MAXN + lo];
            g_mv.to = -1;
            return;
        }
    }
    g_mv.i = (int)(((uint64_t)r * (uint32_t)N) >> 32);
    if (typ >= 4096u - TH_MOVE) { g_mv.j = -1; g_mv.to = (int)rng.next((uint32_t)K); }
    else                        { g_mv.j = (int)rng.next((uint32_t)N); g_mv.to = -1; }
}

// グループ a,b の和が na,nb になったときの max-min を求める。
// 上位 2 / 下位 2 を持っているので普通は O(1)。両方とも a,b だった時だけ K 個を走査する。
static inline float new_spread(const State &s, int a, int b, long long na, long long nb) {
    long long mx = na > nb ? na : nb;
    long long mn = na < nb ? na : nb;
    const int kt = (TOP0 != a && TOP0 != b) ? TOP0 : TOP1;
    if (kt != a && kt != b) { if (s.sum[kt] > mx) mx = s.sum[kt]; }
    else for (int k = 0; k < K; k++) if (k != a && k != b && s.sum[k] > mx) mx = s.sum[k];
    const int kb = (BOT0 != a && BOT0 != b) ? BOT0 : BOT1;
    if (kb != a && kb != b) { if (s.sum[kb] < mn) mn = s.sum[kb]; }
    else for (int k = 0; k < K; k++) if (k != a && k != b && s.sum[k] < mn) mn = s.sum[k];
    return (float)(mx - mn);
}

inline float calc_score(State &s) {
    const int i = g_mv.i;
    if (g_mv.j < 0) {
        const int a = s.grp[i], b = g_mv.to;
        if (a == b) return 0.0f;
        return new_spread(s, a, b, s.sum[a] - AV[i], s.sum[b] + AV[i]) - s.score;
    } else {
        const int j = g_mv.j;
        const int a = s.grp[i], b = s.grp[j];
        if (a == b) return 0.0f;
        const long long d = AV[j] - AV[i];
        return new_spread(s, a, b, s.sum[a] + d, s.sum[b] - d) - s.score;
    }
}

inline void apply_move(State &s) {
    const int i = g_mv.i;
    if (g_mv.j < 0) {
        const int a = s.grp[i], b = g_mv.to;
        s.sum[a] -= AV[i]; s.sum[b] += AV[i]; s.grp[i] = (int8_t)b;
        grp_erase(a, MPOS[i]); grp_insert(b, i);
    } else {
        const int j = g_mv.j;
        const int a = s.grp[i], b = s.grp[j];
        s.sum[a] += AV[j] - AV[i]; s.sum[b] += AV[i] - AV[j];
        s.grp[i] = (int8_t)b; s.grp[j] = (int8_t)a;
        const int pi = MPOS[i], pj = MPOS[j];      // ★先に両方の位置を控える
        grp_erase(a, pi); grp_insert(a, j);
        grp_erase(b, pj); grp_insert(b, i);
    }
    rebuild_rank(s);
}

void read_input() {
    if (scanf("%d %d", &N, &K) == 2 && N >= 1) {
        N = min(N, MAXN); K = min(K, MAXK);
        for (int i = 0; i < N; i++) scanf("%lld", &AV[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 204);
        N = 300; K = 10;
        for (int i = 0; i < N; i++) AV[i] = 1 + (long long)g.next(1000000);
    }
}

void output(const State &s) {
    string r; r.reserve((size_t)N * 3);
    for (int i = 0; i < N; i++) { r += to_string((int)s.grp[i]); r += '\n'; }
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
    init_state(state);  // ★ 初期解構築
    anneal(state, TIME_LIMIT_MS, TEMP_START, TEMP_END);

    output(state);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)state.score);
    fprintf(stderr, "iter = %lld, accept = %lld (%.2f%%), time = %.1f ms\n",
            g_iter, g_accept, 100.0 * (double)g_accept / (double)max(1LL, g_iter), (double)timer.ms());
    // ★差分計算のバグ検出: 下の 2 つがずれていたら calc_score / apply_move が間違っている
    fprintf(stderr, "[check] diff-sum = %.3f, full = %.3f\n",
            (double)state.score, (double)full_score(state));
    return 0;
}
