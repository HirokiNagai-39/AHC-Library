// =============================================================================
//  [貪欲系 01] マルチ配送計画 (multi-vehicle orienteering)
// =============================================================================
//  【問題】
//    2 次元平面上に N 個の地点があり、地点 0 は車庫、地点 i (1<=i<N) には価値 v_i がある。
//    K 台の車がすべて地点 0 を出発し、いくつかの地点を回って地点 0 に戻る。
//    1 台の走行距離は L 以下でなければならない。各地点は高々 1 台が 1 回だけ訪問できる。
//    訪問した地点の価値の合計を最大化せよ。移動距離はユークリッド距離。
//  【入力】
//    N K L
//    x_0 y_0 v_0
//    ...           (N 行。x,y は実数、v は整数。v_0 = 0)
//  【出力】
//    K 行。各行は "m p_1 ... p_m" (その車が訪問する地点を訪問順に。m=0 なら "0" のみ)
//  【スコア】 訪問した地点の価値の合計 (大きいほど良い)。距離制約違反は 0 点。
//  【入力生成方法】
//    N=200, K=4, L=2500 固定。x,y は [0,1000] の一様乱数。v_i は 1..100 の一様整数 (v_0=0)。
//    ※ このファイルは標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     距離の予算内で「おいしい地点だけ拾う」問題 (orienteering)。全部は回れないので、
//     地点の価値だけでも、近さだけでも足りず、その両方を見た「コスパ」で選ぶ必要がある。
//     さらにこの問題の解は「訪問順の列」なので、経路の途中にも地点を差し込める。
//     末尾に足すだけの貪欲より「どこにでも挿入できる貪欲 (挿入貪欲)」が圧倒的に強く、
//     実測でも 末尾追加 15412 に対して 挿入貪欲 22028 だった。
//
//   ● 状態 (State)
//     K 台ぶんの訪問順を 1 本の配列 seq[] に連結して持ち、車 k の区間の長さを len[k] で管理する。
//     車ごとに配列を分けるより State が小さくなり、コピーが速い。
//     ほかに各車の使用距離 used[k] と、地点ごとの訪問済みフラグ vis[] を持つ。
//
//   ● 手の作り方
//     「まだ訪問していない地点 i を、どこかの車の経路のどこかに挿入する」。
//     素直にやると (残り地点 × 全挿入位置) の全組で重いので、挿入位置を
//       ・地点 i の近くにある訪問済み地点の隣 (近傍リストは入力読み込み時に前計算)
//       ・各車の経路の先頭と末尾
//     だけに絞る。遠い地点の隣に挿し込む手はまず選ばれないので、絞っても質はほとんど落ちない。
//     used[k] + 増える距離 > LIMIT になる挿入は候補から外す。
//
//   ● 評価値
//     v_i / (増える距離 + EPS_DIST)^ALPHA  … 「価値 ÷ 寄り道コスト」のコスパ。
//     ALPHA を下げると価値重視、上げると距離重視になる。
//     EPS_DIST は「ほぼ 0 距離の挿入」の評価値が発散するのを防ぐ下駄で、
//     大きくすると「安いが価値も低い地点」に釣られにくくなる (30 が最良だった)。
//
//   ● 差分計算 / 高速化
//     挿入で増える距離は d(prev,i) + d(i,next) - d(prev,next) で O(1)。
//     挿入するたびに、その車の経路にだけ 2-opt と Or-opt を掛けて経路長を縮める。
//     浮いた距離でさらに地点を積めるので、これが効く。
//     最後に評価値へランダムなノイズを掛けた貪欲を時間いっぱい繰り返し (ランダム多点貪欲)、
//     最良解を採用する。1 試行を軽くするほど試行数が増えて強くなる。
//
//   ● つまずきポイント
//     ・貪欲プレイアウト版は候補が (地点 × 車) と多く、1 手ごとのプレイアウトが重すぎて大敗した (6230)。
//       「1 手が重い問題にローリングホライゾンは向かない」典型例。
//     ・挿入位置を絞らないと 1 試行が重く、ランダム多点貪欲の試行数が稼げない。
//
//   ● さらに伸ばすなら
//     ・挿入と同時に「価値の低い地点を抜く」手 (ruin & recreate) を入れる
//     ・焼きなましに移し、「訪問する地点の集合」と「経路」を同時に動かす
//
//  【改善】
//    ・挿入位置を「近傍の訪問済み地点の隣 + 各車の端」に絞り (近傍リストを前計算)、1 手の走査を軽くした
//    ・挿入のたびに 2-opt + Or-opt でその車の経路を短くし、浮いた距離ぶん多くの地点を回れるようにした
//    ・評価値のコスト項を弱めた (ALPHA 1.0 -> 0.7 / EPS_DIST 1 -> 30)
//    seed 0..4 合計: 変更前 35560 -> 変更後 38314 (+7.7%)
//
//  【採用したライブラリ】 greedy/2_insertion_greedy
//    実測比較 (seed 0,1,2 の合計スコア): 1_greedy=15412 / 2_insertion_greedy=22028 / 3_rolling_horizon=6230
// =============================================================================
// =============================================================================
//  過去改変貪欲 (挿入貪欲)   ---  AHC 用 高速テンプレート
// =============================================================================
//  末尾だけでなく操作列の途中にも挿入できる貪欲。1 手ぶんの選択肢が (要素 × 位置) に広がる。
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
//    struct Move  { ... };                     ... 挿入する要素
//    struct State { float score; ... };        ... 状態 (score は必須)
//    init_state(State&)                        ... 初期状態
//    enum_items(const State&, Move*)           ... まだ使っていない要素を全列挙 (0 で終了)
//    seq_len(const State&)                     ... 現在の操作列の長さ (挿入位置は 0〜これ)
//    eval_insert(const State&, Move, int pos)  ... pos に挿入したときの評価値 (不可なら WORST_SCORE)
//    calc_score(const State&, Move, int pos)   ... その挿入で確定するスコア差分
//    apply_insert(State&, Move, int pos)       ... 挿入を実行する (score は触らない)
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
float ALPHA     = 0.7f;    // p1: 評価値のコスト項の効かせ方
float EPS_DIST  = 30.0f;   // p2: 距離の効かせ方をゆるめる下駄 (0 除算よけ兼用)
float NOISE     = 0.30f;   // p3: ランダム貪欲のノイズ幅 (0 なら決定的な貪欲 1 回)
int   CNB       = 6;       // p4: 挿入位置を「近い訪問済み地点」何個の隣に絞るか
int   TWO_PASS  = 2;       // p5: 1 回の挿入ごとに掛ける 2-opt のパス数 (0 で無効)
int   OROPT     = 1;       // p6: Or-opt (1 点の移動) も一緒に掛けるか

void load_params() {
    pick_env("p1", ALPHA);
    pick_env("p2", EPS_DIST);
    pick_env("p3", NOISE);
    pick_env("p4", CNB);
    pick_env("p5", TWO_PASS);
    pick_env("p6", OROPT);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (マルチ配送計画)
// #############################################################################
constexpr int MAXN = 400, MAXK = 8;
constexpr int MAX_CAND = MAXN * MAXK;      // ★候補は (地点 × 車) なのでこの大きさ

int   N, K;
float LIMIT;
static int   VAL[MAXN];
static float D[MAXN * MAXN];
static inline float dist(int i, int j) { return D[i * N + j]; }

// ---- 近傍リスト (前計算): 各地点の「近い順」の地点 ----
constexpr int NNB = 48, MAXCNB = 32;
static int16_t NB[MAXN][NNB];
static int MAXPOS = 1;                     // 1 アイテムあたりの候補挿入位置数 (= 2K + 2*CNB)

struct Move { int16_t to; int8_t car; };

struct State {
    float   score;
    float   used[MAXK];      // 各車の走行距離
    int16_t len[MAXK];       // 各車の訪問数
    int16_t total;           // 訪問した地点の総数
    int16_t seq[MAXN];       // 全車の訪問順を連結して持つ (車 k の区間は base(k)..base(k)+len[k])
    uint8_t vis[MAXN];
};

// ---- 現在の列の索引 (enum_items のたびに O(N) で作り直す) ----
static int    g_base[MAXK];                // 車 k の seq 内の先頭位置
static int8_t g_carof[MAXN];               // 地点 u がどの車か
static int16_t g_idxof[MAXN];              // 地点 u が車の中で何番目か
// ---- 「アイテム i ごとの候補挿入位置」(pos → (車, 位置)) ----
static int8_t  g_pcar[2 * MAXK + 2 * MAXCNB];
static int16_t g_pq  [2 * MAXK + 2 * MAXCNB];
static int     g_pitem = -1;

static inline float pw(float x) {           // powf は重いので指数が 1 / 2 のときは省く
    if (ALPHA == 1.0f) return x;
    if (ALPHA == 2.0f) return x * x;
    return powf(x, ALPHA);
}

// 車 k の位置 q に地点 i を挿入したときの追加距離 (g_base が最新である前提)
static inline float add_cost(const State &s, int k, int q, int i) {
    const int b = g_base[k];
    const int prev = (q == 0)         ? 0 : s.seq[b + q - 1];
    const int next = (q == s.len[k])  ? 0 : s.seq[b + q];
    return dist(prev, i) + dist(i, next) - dist(prev, next);
}

void init_state(State &s) {
    s.score = 0.0f; s.total = 0;
    for (int k = 0; k < K; k++) { s.used[k] = 0.0f; s.len[k] = 0; }
    memset(s.vis, 0, sizeof(uint8_t) * (size_t)N);
    s.vis[0] = 1;
}

// 実際に列へ差し込む
static inline void do_insert(State &s, int k, int q, int to) {
    s.used[k] += add_cost(s, k, q, to);
    const int g = g_base[k] + q;
    memmove(s.seq + g + 1, s.seq + g, sizeof(int16_t) * (size_t)(s.total - g));
    s.seq[g] = (int16_t)to; s.len[k]++; s.total++; s.vis[to] = 1;
}

// 車 k の経路を 2-opt で短くする (縮んだぶん used[k] が減り、その車にもっと地点を積める)
static inline void two_opt(State &s, int k) {
    const int n = s.len[k];
    if (n < 3 || TWO_PASS <= 0) return;
    int16_t *r = s.seq + g_base[k];
    float gain = 0.0f;
    for (int pass = 0; pass < TWO_PASS; pass++) {
        bool imp = false;
        for (int i = 0; i + 1 < n; i++) {
            const int a = (i == 0) ? 0 : r[i - 1];
            for (int j = i + 1; j < n; j++) {
                const int c = (j == n - 1) ? 0 : r[j + 1];
                const float delta = dist(a, r[j]) + dist(r[i], c) - dist(a, r[i]) - dist(r[j], c);
                if (delta < -1e-3f) {                       // 交差を解く
                    for (int x = i, y = j; x < y; x++, y--) { const int16_t t = r[x]; r[x] = r[y]; r[y] = t; }
                    gain += delta; imp = true;
                }
            }
        }
        for (int i = 0; i < n && OROPT; i++) {                // Or-opt: 1 点を別の位置へ動かす
            const int u = r[i];
            const int p0 = (i == 0) ? 0 : r[i - 1], q0 = (i == n - 1) ? 0 : r[i + 1];
            const float rem = dist(p0, u) + dist(u, q0) - dist(p0, q0);
            int ba = -2; float bd = -1e-3f;
            for (int a = -1; a < n; a++) {
                if (a == i - 1 || a == i) continue;
                const int x = (a < 0) ? 0 : r[a], y = (a + 1 >= n) ? 0 : r[a + 1];
                const float d = dist(x, u) + dist(u, y) - dist(x, y) - rem;
                if (d < bd) { bd = d; ba = a; }
            }
            if (ba == -2) continue;
            if (ba < i) {                                    // 前へ動かす
                memmove(r + ba + 2, r + ba + 1, sizeof(int16_t) * (size_t)(i - ba - 1));
                r[ba + 1] = (int16_t)u;
            } else {                                         // 後ろへ動かす
                memmove(r + i, r + i + 1, sizeof(int16_t) * (size_t)(ba - i));
                r[ba] = (int16_t)u;
            }
            gain += bd; imp = true;
        }
        if (!imp) break;
    }
    s.used[k] += gain;
}

// ---- 挿入貪欲用 ----
inline int enum_items(const State &s, Move *out) {
    int b = 0;                                          // 索引を作り直す
    for (int k = 0; k < K; k++) {
        g_base[k] = b;
        for (int q = 0; q < s.len[k]; q++) { const int u = s.seq[b + q]; g_carof[u] = (int8_t)k; g_idxof[u] = (int16_t)q; }
        b += s.len[k];
    }
    g_pitem = -1;
    int m = 0;
    for (int i = 1; i < N; i++) if (!s.vis[i]) { out[m].to = (int16_t)i; out[m].car = 0; m++; }
    return m;
}

// 地点 i の候補挿入位置を作る (アイテムが変わったときだけ。状態は走査中に変わらない)
static inline void build_cands(const State &s, int i) {
    if (g_pitem == i) return;
    g_pitem = i;
    int n = 0;
    for (int k = 0; k < K; k++) {                        // 各車の先頭と末尾は必ず候補に入れる
        g_pcar[n] = (int8_t)k; g_pq[n] = 0;              n++;
        g_pcar[n] = (int8_t)k; g_pq[n] = s.len[k];       n++;
    }
    int c = 0;
    const int16_t *nb = NB[i];
    for (int t = 0; t < NNB && c < CNB; t++) {           // 近い順に「訪問済みの地点の隣」を候補にする
        const int u = nb[t];
        if (!s.vis[u] || u == 0) continue;
        const int k = g_carof[u], q = g_idxof[u];
        g_pcar[n] = (int8_t)k; g_pq[n] = (int16_t)q;       n++;   // u の直前
        g_pcar[n] = (int8_t)k; g_pq[n] = (int16_t)(q + 1); n++;   // u の直後
        c++;
    }
    while (n < MAXPOS) { g_pcar[n] = -1; g_pq[n] = 0; n++; }      // 余りは無効
}

inline int seq_len(const State &s) { (void)s; return MAXPOS - 1; }

inline float eval_insert(const State &s, const Move &mv, int pos) {
    build_cands(s, mv.to);
    const int k = g_pcar[pos];
    if (k < 0) return WORST_SCORE;
    const float add = add_cost(s, k, g_pq[pos], mv.to);
    if (s.used[k] + add > LIMIT) return WORST_SCORE;
    return (float)VAL[mv.to] / pw(add + EPS_DIST);
}
inline float calc_score(const State &s, const Move &mv) { (void)s; return (float)VAL[mv.to]; }
inline float calc_score(const State &s, const Move &mv, int pos) { (void)pos; return calc_score(s, mv); }
inline void apply_insert(State &s, const Move &mv, int pos) {
    build_cands(s, mv.to);
    const int k = g_pcar[pos];
    do_insert(s, k, g_pq[pos], mv.to);
    two_opt(s, k);
}

// ---- 貪欲 / プレイアウト用 (末尾追加のみ) ----
inline int enum_moves(const State &s, Move *out) {
    int b = 0;
    for (int k = 0; k < K; k++) { g_base[k] = b; b += s.len[k]; }
    int m = 0;
    for (int i = 1; i < N; i++) if (!s.vis[i])
        for (int k = 0; k < K; k++) { out[m].to = (int16_t)i; out[m].car = (int8_t)k; m++; }
    return m;
}
inline float eval_move(const State &s, const Move &mv) {
    const float add = add_cost(s, mv.car, s.len[mv.car], mv.to);
    if (s.used[mv.car] + add > LIMIT) return WORST_SCORE;
    return (float)VAL[mv.to] / pw(add + EPS_DIST);
}
inline void apply_move(State &s, const Move &mv) { do_insert(s, mv.car, s.len[mv.car], mv.to); two_opt(s, mv.car); }

void read_input() {
    static float px[MAXN], py[MAXN];
    if (scanf("%d %d %f", &N, &K, &LIMIT) == 3 && N >= 2) {
        N = min(N, MAXN); K = min(K, MAXK);
        for (int i = 0; i < N; i++) scanf("%f %f %d", &px[i], &py[i], &VAL[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 11);
        N = 200; K = 4; LIMIT = 2500.0f;
        for (int i = 0; i < N; i++) { px[i] = g.nextf() * 1000.0f; py[i] = g.nextf() * 1000.0f; VAL[i] = 1 + (int)g.next(100); }
        VAL[0] = 0;
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            const float dx = px[i] - px[j], dy = py[i] - py[j];
            D[i * N + j] = sqrtf(dx * dx + dy * dy);
        }
    // ---- 近傍リストと候補位置数の前計算 ----
    if (CNB > MAXCNB) CNB = MAXCNB;
    if (CNB < 1) CNB = 1;
    MAXPOS = 2 * K + 2 * CNB;
    static int ord[MAXN];
    for (int i = 0; i < N; i++) {
        int c = 0;
        for (int j = 1; j < N; j++) if (j != i) ord[c++] = j;
        const int take = min(NNB, c);
        partial_sort(ord, ord + take, ord + c, [&](int a, int b) { return D[i * N + a] < D[i * N + b]; });
        for (int t = 0; t < take; t++) NB[i][t] = (int16_t)ord[t];
        for (int t = take; t < NNB; t++) NB[i][t] = (int16_t)ord[c ? c - 1 : 0];
    }
}

void output(const State &s) {
    int b = 0;
    for (int k = 0; k < K; k++) {
        printf("%d", (int)s.len[k]);
        for (int q = 0; q < s.len[k]; q++) printf(" %d", (int)s.seq[b + q]);
        printf("\n");
        b += s.len[k];
    }
}

float replay_true_score(const State &s) {
    static uint8_t seen[MAXN];
    memset(seen, 0, sizeof(uint8_t) * (size_t)N);
    float total = 0.0f; int b = 0;
    for (int k = 0; k < K; k++) {
        float used = 0.0f; int prev = 0;
        for (int q = 0; q < s.len[k]; q++) {
            const int c = s.seq[b + q];
            if (c <= 0 || c >= N || seen[c]) { fprintf(stderr, "[error] 不正な訪問 %d\n", c); return -1.0f; }
            seen[c] = 1; used += dist(prev, c); total += (float)VAL[c]; prev = c;
        }
        used += dist(prev, 0);
        if (used > LIMIT + 1e-2f) { fprintf(stderr, "[error] 車 %d の距離超過 %.1f\n", k, used); return -1.0f; }
        b += s.len[k];
    }
    return total;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
// 7. 過去改変貪欲 (挿入貪欲) 本体
// =============================================================================
//  普通の貪欲は「解の末尾」にしか操作を足せないが、こちらは
//  「操作列のどこにでも挿入できる」。1 手ぶんの探索空間が (要素数 × 挿入位置) に広がるので、
//  末尾追加だけの貪欲より確実に良い解が出る (TSP の最近挿入法と同じ考え方)。
//  計算量は 1 挿入あたり O(残り要素数 × 列の長さ) なので、そこが重いなら
//  挿入位置を「近い方から数箇所だけ」に絞ると良い。
static Move  g_item[MAX_CAND];
static State g_cur, g_best;

static void run_insertion(State &s, float noise) {
    init_state(s);
    while (true) {
        const int m = enum_items(s, g_item);
        if (m == 0) break;
        const int L = seq_len(s);
        // ---- (要素, 挿入位置) の全組から一番良いものを 1 パスで拾う ----
        int   bi = -1, bp = 0;
        float bv = WORST_SCORE;
        for (int k = 0; k < m; k++) {
            for (int p = 0; p <= L; p++) {
                float e = eval_insert(s, g_item[k], p);
                if (e == WORST_SCORE) continue;          // 挿入不可
                if (noise > 0.0f) e *= 1.0f + noise * (rng.nextf() - 0.5f);
                if (is_better(e, bv)) { bv = e; bi = k; bp = p; }
            }
        }
        if (bi < 0) break;                                // どこにも入らない
        s.score += calc_score(s, g_item[bi], bp);
        apply_insert(s, g_item[bi], bp);
        g_steps++;
    }
    g_trials++;
}

void insertion_greedy(float deadline_ms) {
    run_insertion(g_best, 0.0f);
    if (NOISE <= 0.0f) return;
    while (timer.ms() < deadline_ms) {
        run_insertion(g_cur, NOISE);
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

    insertion_greedy(TIME_LIMIT_MS);
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
