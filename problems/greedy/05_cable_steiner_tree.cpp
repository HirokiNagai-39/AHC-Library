// =============================================================================
//  [貪欲系 05] ケーブル配線 (グリッド上のシュタイナー木)
// =============================================================================
//  【問題】
//    H×W のグリッドがある。セル (i,j) にケーブルを敷設するコストは c_ij。
//    グリッド上に T 個の端子セルがある。すべての端子が「敷設したセルだけを通って
//    4 近傍で行き来できる」ようにケーブルを敷設したい。
//    敷設したセルのコスト合計を最小化せよ (端子セル自身も敷設が必要)。
//  【入力】
//    H W T
//    c_00 ... c_0(W-1)   (H 行)
//    ti tj               (T 行。端子の座標)
//  【出力】
//    H 行 W 列の 0/1。敷設したセルを 1、していないセルを 0 とする。
//  【スコア】 敷設したセルのコスト合計 (小さいほど良い)。非連結なら 0 点扱い。
//  【入力生成方法】
//    H=W=30, T=20 固定。c_ij は 1..9 の一様整数。端子はすべて異なるセルから一様ランダムに選ぶ。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     端子を全部つなぐ最小コストの配線 (グリッド上のシュタイナー木) は NP 困難だが、
//     「今の配線からいちばん安くつながる端子を選んで、その最短経路を敷く」を繰り返す
//     最近端子接続法が定番で、実装も素直。
//     局所探索が組みにくい (セルを 1 個外すと連結性が壊れる) ので、貪欲系が向く問題の典型。
//
//   ● 状態 (State)
//     どのセルを敷設したかのフラグ、どの端子がつながったか、
//     そして「敷設済みセルからの最短距離 dist[] とその親 par[]」。
//     距離と親を State に持つのが高速化の鍵 (下記)。
//
//   ● 手の作り方
//     「次にどの端子をつなぐか」を選ぶ。
//     敷設済みセルを全部 距離 0 の始点にした多始点ダイクストラを 1 回回せば、
//     全端子への接続コストが同時に求まる。あとは一番安い端子を選び、
//     親配列を終点から辿ってその経路上のセルを敷設するだけ。
//
//   ● 評価値
//     その端子の接続コスト (= ダイクストラの距離)。最小化なので小さいほど良い。
//     敷設済みのセルを通るコストは 0 にしてあるので、既存の配線に相乗りする経路が自然に選ばれる。
//
//   ● 差分計算 / 高速化
//     ここが本題。毎回ダイクストラを全計算すると重いので、
//       ・距離配列と親配列を State に持ち、セルを敷設したときは
//         「新しく敷設したセルだけを距離 0 の始点にした差分ダイクストラ」で更新する。
//         既存の距離は上界として正しいので、改善する範囲だけ触れば済む
//       ・セルのコストが 1..9 と小さいので、優先度付きキューではなく Dial 法 (バケツ) を使う
//     この 2 つで 1 試行のコストが約 1/40 になり、試行数が 13 回から 570 回に増えた。
//
//   ● つまずきポイント
//     ・多始点ダイクストラの「入るときにコストを払う」向きを間違えやすい。
//       敷設済みセルは 0、未敷設セルは CST[u] を、そのセルに入るときに払う。
//     ・ランダム多点貪欲にするには揺らす場所が要る。この問題では
//       「根にする端子」「等コスト経路が複数あるときの選び方」「探索用コストの微小な揺らし」の 3 つ。
//       スコアは必ず「実際に敷設したセルの本当のコスト」で積むこと (揺らした値で積むとずれる)。
//
//   ● さらに伸ばすなら
//     ・キーパス局所探索 (木の枝を 1 本外して張り直す) を足す。
//       ただし試作した範囲では現状のスコアに届かなかった (386/418/424 対 381/410/417)
//     ・時間を 20 秒に伸ばしても改善しなかったので、貪欲系ではこの辺りが飽和点
//
//  【改善】
//    ダイクストラを毎回の全計算 (priority_queue) から Dial 法 + State が距離を持つ差分更新に変更し、
//    1 試行のコストを約 1/40 にした (試行数 13 -> 570)。
//    さらに根の端子・等コスト経路・探索コストをランダム化して、多点貪欲を実質的に機能させた。
//    seed 0..4 合計: 変更前 1973 -> 変更後 1956 (-0.9%)   ※seed 5..9 でも 2014 -> 1980 (-1.7%)
//
//  【採用したライブラリ】 greedy/3_rolling_horizon
//    実測比較 (seed 0,1,2 の合計スコア): 1_greedy=1237 / 3_rolling_horizon=1222
// =============================================================================
// =============================================================================
//  貪欲プレイアウト (ローリングホライゾン)   ---  AHC 用 高速テンプレート
// =============================================================================
//  各候補手について先を貪欲でプレイアウトし、結果が最良の手を採用する。1 と同じインターフェース。
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
//    ※ 3 は 1 と全く同じインターフェース。1 で書いたものをそのまま使える。
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
float ALPHA     = 1.0f;    // p1: (この問題では未使用)
float EPS_DIST  = 1.0f;    // p2: 0 除算よけ
int   HORIZON   = -1;      // p3: プレイアウトで何手先まで進めるか (-1 なら最後まで)
float NOISE     = 0.05f;   // p4: プレイアウトのノイズ幅
float JITTER    = 0.35f;   // p5: 経路探索コストの揺らし幅 (多点貪欲の多様性)
int   RAND_ROOT = 1;       // p6: 根にする端子をランダムに選ぶか (0/1)

void load_params() {
    pick_env("p1", ALPHA);
    pick_env("p2", EPS_DIST);
    pick_env("p3", HORIZON);
    pick_env("p4", NOISE);
    pick_env("p5", JITTER);
    pick_env("p6", RAND_ROOT);
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
// (この手法は探索設定より「評価関数」と「プレイアウトの長さ」が効く)

// 統計 (デバッグ用。不要なら消して良い)
static ll g_trials = 0, g_steps = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (ケーブル配線 / シュタイナー木)
// #############################################################################
constexpr int MAXH = 40, MAXW = 40, MAXC = MAXH * MAXW, MAXT = 64;
constexpr int MAX_CAND = MAXT;             // ★候補は「次に接続する端子」
constexpr int INF16   = 1 << 29;
constexpr int JSCALE  = 8;                 // 揺らしを整数で表すための倍率
constexpr int MAXD    = MAXC * 10 * JSCALE * 2;   // Dial 法のバケツ数 (距離の上界)

int H, W, TN;
static int CST[MAXC], TERM[MAXT];
static int NB[MAXC][4], NBN[MAXC];         // 4 近傍の前計算 (毎回の割り算・境界判定をなくす)
static int PC[MAXC];                       // 経路探索に使うコスト (1 試行ごとに揺らす)

struct Move { int16_t t; };

// ★State が「敷設済みセルからの多始点距離とその親」を自分で持つ。
//   セルを敷設したときは、新しく敷設したセルだけを始点にした差分ダイクストラで更新する。
struct State {
    float   score;
    int32_t nconn;
    int32_t dist[MAXC];
    int16_t par[MAXC];
    uint8_t laid[MAXC];
    uint8_t conn[MAXT];
};

// ---- Dial (バケツ) 法による多始点ダイクストラの差分更新 ----
static int  g_bhead[MAXD];
static int  g_pv[MAXC * 8], g_pnx[MAXC * 8], g_psz;
static int  g_src[MAXC], g_srcn;
static bool g_binit = false;

static inline void bucket_push(int d, int v) { g_pv[g_psz] = v; g_pnx[g_psz] = g_bhead[d]; g_bhead[d] = g_psz++; }

// g_src (新たに敷設したセル) を距離 0 の始点として距離を更新する。
// 既存の dist は上界として正しいので、改善する範囲だけ触れば済む (差分ダイクストラ)。
// 同コストの経路が来たときは 1/2 の確率で親を差し替える (等コスト経路のランダム化)。
static void dij_update(State &s) {
    if (!g_binit) { for (int i = 0; i < MAXD; i++) g_bhead[i] = -1; g_binit = true; }
    g_psz = 0;
    int dmax = 0;
    for (int i = 0; i < g_srcn; i++) { const int v = g_src[i]; s.dist[v] = 0; s.par[v] = -1; bucket_push(0, v); }
    for (int d = 0; d <= dmax; d++) {
        while (g_bhead[d] != -1) {
            const int e = g_bhead[d]; g_bhead[d] = g_pnx[e];
            const int v = g_pv[e];
            if (s.dist[v] != d) continue;
            const int n = NBN[v];
            for (int k = 0; k < n; k++) {
                const int u = NB[v][k];
                const int nd = d + (s.laid[u] ? 0 : PC[u]);
                if (nd < s.dist[u]) {
                    s.dist[u] = nd; s.par[u] = (int16_t)v;
                    bucket_push(nd, u);
                    if (nd > dmax) dmax = nd;
                } else if (nd == s.dist[u] && nd > 0 && (rng.next() & 1u)) {
                    s.par[u] = (int16_t)v;                 // 等コストの別経路を採用 (木の形を散らす)
                }
            }
        }
    }
}

// 1 試行ぶんの探索コストを作る (JITTER=0 なら決定的)
static void make_pcost() {
    const int C = H * W;
    if (JITTER <= 0.0f) { for (int i = 0; i < C; i++) PC[i] = CST[i] * JSCALE; return; }
    for (int i = 0; i < C; i++) {
        const float b = (float)(CST[i] * JSCALE);
        int v = (int)(b + b * JITTER * (rng.nextf() - 0.5f) + 0.5f);
        PC[i] = v < 1 ? 1 : v;
    }
}

void init_state(State &s) {
    const int C = H * W;
    make_pcost();
    memset(s.laid, 0, sizeof(s.laid));
    memset(s.conn, 0, sizeof(s.conn));
    for (int i = 0; i < C; i++) { s.dist[i] = INF16; s.par[i] = -1; }
    const int r = (RAND_ROOT > 0) ? (int)rng.next((uint32_t)TN) : 0;    // 根にする端子
    s.laid[TERM[r]] = 1; s.conn[r] = 1; s.nconn = 1;
    s.score = (float)CST[TERM[r]];
    g_srcn = 0; g_src[g_srcn++] = TERM[r];
    dij_update(s);
}

inline int enum_moves(const State &s, Move *out) {
    if (s.nconn >= TN) return 0;
    int m = 0;
    for (int t = 0; t < TN; t++) if (!s.conn[t]) out[m++].t = (int16_t)t;
    return m;
}
// 評価値 = 接続コスト (最小化なので小さいほど良い)
inline float eval_move(const State &s, const Move &mv) {
    const int d = s.dist[TERM[mv.t]];
    return (d >= INF16) ? WORST_SCORE : (float)d;
}
// ★スコアは「揺らしたコスト」ではなく、経路を辿って本当のコストを足し上げる
inline float calc_score(const State &s, const Move &mv) {
    int v = TERM[mv.t], tot = 0;
    while (v >= 0 && !s.laid[v]) { tot += CST[v]; v = s.par[v]; }
    return (float)tot;
}
inline void apply_move(State &s, const Move &mv) {
    int v = TERM[mv.t];                            // 親を辿って経路上のセルを敷設する
    g_srcn = 0;
    while (v >= 0 && !s.laid[v]) { s.laid[v] = 1; g_src[g_srcn++] = v; v = s.par[v]; }
    s.conn[mv.t] = 1; s.nconn++;
    if (g_srcn) dij_update(s);                     // 新しく敷設したセルぶんだけ距離を更新
}

void read_input() {
    if (scanf("%d %d %d", &H, &W, &TN) == 3 && H >= 1) {
        H = min(H, MAXH); W = min(W, MAXW); TN = min(TN, MAXT);
        for (int i = 0; i < H * W; i++) scanf("%d", &CST[i]);
        for (int t = 0; t < TN; t++) { int a, b; scanf("%d %d", &a, &b); TERM[t] = a * W + b; }
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 55);
        H = 30; W = 30; TN = 20;
        for (int i = 0; i < H * W; i++) CST[i] = 1 + (int)g.next(9);
        static uint8_t used[MAXC]; memset(used, 0, sizeof(used));
        for (int t = 0; t < TN;) { const int c = (int)g.next((uint32_t)(H * W)); if (!used[c]) { used[c] = 1; TERM[t++] = c; } }
    }
    for (int i = 0; i < H; i++) for (int j = 0; j < W; j++) {   // 4 近傍の前計算
        const int v = i * W + j; int n = 0;
        if (i > 0)     NB[v][n++] = v - W;
        if (i < H - 1) NB[v][n++] = v + W;
        if (j > 0)     NB[v][n++] = v - 1;
        if (j < W - 1) NB[v][n++] = v + 1;
        NBN[v] = n;
    }
}

void output(const State &s) {
    string r; r.reserve((size_t)(H * (W + 1)));
    for (int i = 0; i < H; i++) { for (int j = 0; j < W; j++) r += (s.laid[i * W + j] ? '1' : '0'); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const State &s) {
    const int C = H * W;
    float total = 0.0f;
    for (int i = 0; i < C; i++) if (s.laid[i]) total += (float)CST[i];
    // 連結性チェック (端子 0 から敷設セルだけを辿って全端子に届くか)
    static int st[MAXC]; static uint8_t vis[MAXC];
    memset(vis, 0, sizeof(uint8_t) * (size_t)C);
    int sp = 0; st[sp++] = TERM[0]; vis[TERM[0]] = 1;
    while (sp) {
        const int v = st[--sp], vi = v / W, vj = v % W;
        const int dx[4] = {-1, 1, 0, 0}, dy[4] = {0, 0, -1, 1};
        for (int k = 0; k < 4; k++) {
            const int ni = vi + dx[k], nj = vj + dy[k];
            if (ni < 0 || ni >= H || nj < 0 || nj >= W) continue;
            const int u = ni * W + nj;
            if (s.laid[u] && !vis[u]) { vis[u] = 1; st[sp++] = u; }
        }
    }
    for (int t = 0; t < TN; t++) if (!vis[TERM[t]]) { fprintf(stderr, "[error] 端子 %d が非連結\n", t); return -1.0f; }
    return total;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
// 7. 貪欲プレイアウト (ローリングホライゾン) 本体
// =============================================================================
//  1 手を決めるのに「その手を打った後を貪欲で HORIZON 手ぶん進めてみて、
//  結果が一番良かった手」を採用する。目先の評価値だけでは分からない
//  「この手を打つと後で詰む」を検出できるので、単純な貪欲よりかなり強い。
//  コストは (候補数 × プレイアウトの長さ) 倍になるので、時間配分に注意する。
static Move  g_cand[MAX_CAND];
static Move  g_cand2[MAX_CAND];
static State g_cur, g_best, g_work;

// s から貪欲で最大 h 手進めて、到達したスコアを返す (h < 0 なら最後まで)
static float playout(State &s, int h, float noise) {
    for (int step = 0; h < 0 || step < h; step++) {
        const int m = enum_moves(s, g_cand2);
        if (m == 0) break;
        int   bi = -1;
        float bv = WORST_SCORE;
        for (int k = 0; k < m; k++) {
            float e = eval_move(s, g_cand2[k]);
            if (e == WORST_SCORE) continue;
            if (noise > 0.0f) e *= 1.0f + noise * (rng.nextf() - 0.5f);
            if (is_better(e, bv)) { bv = e; bi = k; }
        }
        if (bi < 0) break;
        s.score += calc_score(s, g_cand2[bi]);
        apply_move(s, g_cand2[bi]);
        g_steps++;
    }
    return s.score;
}

// 1 回ぶん (最初から最後まで) を回す
static void run_rh(State &g_cur, float noise, float deadline_ms) {
    init_state(g_cur);
    while (true) {
        const int m = enum_moves(g_cur, g_cand);
        if (m == 0) break;
        if (m == 1) {                                   // 選択の余地なし
            if (eval_move(g_cur, g_cand[0]) == WORST_SCORE) break;
            g_cur.score += calc_score(g_cur, g_cand[0]);
            apply_move(g_cur, g_cand[0]);
            continue;
        }
        // ---- 各候補について「打った後を貪欲で進めた結果」を比べる ----
        int   bi = -1;
        float bv = WORST_SCORE;
        for (int k = 0; k < m; k++) {
            if (eval_move(g_cur, g_cand[k]) == WORST_SCORE) continue;   // 実行不可の手
            g_work = g_cur;                             // ★State のコピー
            g_work.score += calc_score(g_work, g_cand[k]);
            apply_move(g_work, g_cand[k]);
            const float v = playout(g_work, HORIZON, noise);
            if (is_better(v, bv)) { bv = v; bi = k; }
            if (timer.ms() > deadline_ms) break;        // 時間切れなら今までの中で最良を採用
        }
        if (bi < 0) break;
        g_cur.score += calc_score(g_cur, g_cand[bi]);
        apply_move(g_cur, g_cand[bi]);
    }
    g_trials++;
}

void rolling_horizon(float deadline_ms) {
    run_rh(g_best, 0.0f, deadline_ms);                 // まず決定的に 1 回
    if (NOISE <= 0.0f) return;
    while (timer.ms() < deadline_ms) {                 // 時間が余ったらノイズ付きで繰り返す
        run_rh(g_cur, NOISE, deadline_ms);
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

    rolling_horizon(TIME_LIMIT_MS);
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
