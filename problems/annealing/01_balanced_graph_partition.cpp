// =============================================================================
//  [焼きなまし系 01] 均等グラフ分割 (balanced graph partitioning)
// =============================================================================
//  【問題】
//    N 頂点・E 辺の重み付き無向グラフがある。頂点を K 個のグループに分ける。
//    各グループの頂点数はちょうど N/K でなければならない (N は K の倍数)。
//    両端が別グループになった辺の重みの合計 (カット重み) を最小化せよ。
//  【入力】
//    N K E
//    u v w   (E 行、0<=u,v<N, 重み w)
//  【出力】
//    N 行。頂点 i が属するグループ番号 (0..K-1)。
//  【スコア】 カット重みの合計 (小さいほど良い)。グループサイズが不均等なら 0 点。
//  【入力生成方法】
//    N=600, K=6, E=6000 固定。まず頂点を 6 個の「潜在クラスタ」にランダムに割り振り、
//    各辺は 70% の確率で同じクラスタ内の 2 頂点、30% の確率で全体から一様に 2 頂点を選ぶ
//    (重複辺・自己ループは張り直す)。重み w は 1..10 の一様整数。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「グループサイズがちょうど N/K」という制約をどう扱うかが最初の分かれ道。
//     ペナルティ項にする手もあるが、この問題では
//       近傍を「グループが違う 2 頂点を入れ替える」だけに限る
//     ことで、サイズが常に自動で保たれる。制約を近傍の設計で消してしまうと、
//     ペナルティの重み調整が要らず、探索が制約違反の海をさまようこともない。
//     初期解も「シャッフルして順番に配る」だけで完全に均等になる。
//
//   ● 状態 (State)
//     頂点 -> グループの割当 grp[]、および
//     CNT[v][g] = 「頂点 v からグループ g へ伸びる辺の重みの合計」。
//     CNT があると差分が O(1) で出る (下記)。
//
//   ● 手の作り方
//     頂点 u を選び、u とは違うグループから頂点 v を選んで入れ替える。
//     確率 P_NB で u を「辺の端点」から取る (カットされている辺を直しやすい)、
//     残りはランダムな頂点。v は必ず「u と違うグループ」から引き直すので、無駄打ちが出ない。
//
//   ● 評価値
//     カット重み (両端が別グループの辺の重みの合計) そのもの。最小化。
//     制約は近傍で保証されているので、ペナルティ項は無い。
//
//   ● 差分計算 / 高速化
//     u を a -> b、v を b -> a に移すときのカット重みの変化は
//       (CNT[u][a] - CNT[u][b]) + (CNT[v][b] - CNT[v][a]) + 補正
//     で O(1)。補正は辺 (u,v) が存在する場合のぶんで、
//     入れ替えても両端は別グループのままなのでカット状態は変わらない (打ち消す)。
//     辺の有無はビットセットで判定し、多重辺は隣接リストにまとめてある。
//     この工夫で反復回数が 10.6M から 106M へ 10 倍になった。
//
//   ● つまずきポイント
//     ・辺 (u,v) の補正を忘れると差分が二重に効いてしまう。ここは [verify] で検出できる。
//     ・v をランダムに引くと 1/K の確率で「同じグループ」を引いて何も起きない。
//       引き直す方が素直に速い。
//
//   ● さらに伸ばすなら
//     ・この問題は既にほぼ最適解に届いており (温度やスタート回数を桁で振っても seed 0 は常に 8696 に収束)、
//       伸びしろがほとんど無い。反復を 10 倍にしても -0.1% しか動かなかった。
//     ・伸ばすなら「1 頂点を別グループへ移す + サイズのペナルティ」に近傍を広げ、
//       サイズ制約を一時的に破って谷を越えられるようにする方向
//
//  【改善】
//    CNT[v][g] を持って差分計算を O(deg) から O(1) にし、v を必ず別グループから引くようにした
//    (無駄打ちが消え、反復回数が 10 倍)。多重辺は隣接リストにまとめ、辺の有無はビットセットで判定。
//    seed 0..4 合計: 変更前 43203 -> 変更後 43156 (-0.11%、小さいほど良い)
//    ※ seed 0..9 の 10 個で悪化ゼロ・4 個で改善。この問題は既にほぼ最適解に届いており伸びしろが小さい。
//
//  【採用したライブラリ】 simulated annealing/4_multi_start_annealing
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=26253 / 2_hill_climbing_kick=26288 / 3_simulated_annealing=26218 / 4_multi_start_annealing=26203 / 5_iterated_annealing=26214
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
float TEMP_START = 30.0f;   // p1: 開始温度
float TEMP_END   = 0.5f;   // p2: 終了温度
int   NUM_STARTS = 4;    // p3: スタート回数 (時間をこの数で等分する)
float P_NB       = 0.5f;   // p4: 辺の両端を狙う確率 (1-p はランダムな 2 頂点)

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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (均等グラフ分割)
// #############################################################################
constexpr int MAXN = 800, MAXK = 16, MAXE = 12000;
constexpr int WBIT = MAXN / 64;            // 隣接ビットセットの 1 頂点あたりのワード数

int N, K, E;
static int  EU[MAXE], EV[MAXE], EW[MAXE];
// ---- 多重辺をまとめた隣接リスト (CSR、相手番号でソート済み) ----
static int  ABEG[MAXN + 1], ATO[2 * MAXE], AW[2 * MAXE];
// ---- 隣接ビットセット: (u,v) 間に辺があるかを 1 回のロードで判定する ----
static uint64_t ABIT[MAXN][WBIT];
// ---- CNT[v][g] = 頂点 v からグループ g へ伸びる辺の重みの合計 ----
//      これを持つと 1 手の差分が O(1) で求まる (元の実装は O(deg))。
//      State には入れず大域に置き、init_state で作り直す (best のコピーを軽く保つため)。
static int  CNT[MAXN][MAXK];

// ★必須: 状態
struct State {
    float  score;              // ★必須: カット重み
    int8_t grp[MAXN];
};

// 直前に modify() が選んだ近傍 (頂点 u と v のグループを入れ替える)
struct Move { int u, v; };
static Move g_mv;

float full_score(const State &s) {
    float c = 0.0f;
    for (int e = 0; e < E; e++) if (s.grp[EU[e]] != s.grp[EV[e]]) c += (float)EW[e];
    return c;
}

// u と v の間の辺の重み合計 (多重辺があるのでまとめた値を二分探索で引く)
inline int edge_w(int u, int v) {
    int lo = ABEG[u], hi = ABEG[u + 1];
    while (lo < hi) { const int m = (lo + hi) >> 1; if (ATO[m] < v) lo = m + 1; else hi = m; }
    return (lo < ABEG[u + 1] && ATO[lo] == v) ? AW[lo] : 0;
}

// CNT を今の割当から作り直す
static void rebuild_cnt(const State &s) {
    memset(CNT, 0, sizeof(int) * (size_t)N * MAXK);
    for (int e = 0; e < E; e++) {
        CNT[EU[e]][s.grp[EV[e]]] += EW[e];
        CNT[EV[e]][s.grp[EU[e]]] += EW[e];
    }
}

void init_state(State &s) {
    static int idx[MAXN];
    for (int i = 0; i < N; i++) idx[i] = i;
    for (int i = N - 1; i > 0; i--) swap(idx[i], idx[rng.next(i + 1)]);
    for (int i = 0; i < N; i++) s.grp[idx[i]] = (int8_t)(i % K);   // ちょうど均等になるように配る
    s.score = full_score(s);
    rebuild_cnt(s);
}

// ★必須: 近傍を 1 つ選ぶ (グループが違う 2 頂点を選んで入れ替える → サイズは常に均等のまま)
//   確率 P_NB でカット辺の端点を u に取り、残りはランダムな頂点。
//   v は必ず「u と違うグループ」から取る (同じグループだと差分 0 の無駄打ちになるため)。
inline void modify(State &s) {
    int u;
    if (rng.nextf() < P_NB) {
        const int e = (int)rng.next((uint32_t)E);
        u = (rng.next() & 1) ? EU[e] : EV[e];
    } else {
        u = (int)rng.next((uint32_t)N);
    }
    const int a = s.grp[u];
    int v;
    do { v = (int)rng.next((uint32_t)N); } while (s.grp[v] == a);
    g_mv.u = u; g_mv.v = v;
}

// ★必須: スコア差分  (CNT を使って O(1))
//   u を a→b、v を b→a に動かすと
//     Δ = (CNT[u][a]-CNT[u][b]) + (CNT[v][b]-CNT[v][a]) + 2*w(u,v)
//   最後の項は「辺 (u,v) は入れ替えてもカットのままなのに 2 回引かれてしまう」ぶんの戻し。
inline float calc_score(State &s) {
    const int u = g_mv.u, v = g_mv.v;
    const int a = s.grp[u], b = s.grp[v];
    const int *cu = CNT[u], *cv = CNT[v];
    int d = (cu[a] - cu[b]) + (cv[b] - cv[a]);
    if ((ABIT[u][(unsigned)v >> 6] >> ((unsigned)v & 63)) & 1ull) d += 2 * edge_w(u, v);
    return (float)d;
}

// ★必須: 採用が決まった近傍を反映する (CNT も O(deg) で更新)
inline void apply_move(State &s) {
    const int u = g_mv.u, v = g_mv.v;
    const int a = s.grp[u], b = s.grp[v];
    for (int e = ABEG[u], ee = ABEG[u + 1]; e < ee; e++) { int *c = CNT[ATO[e]]; c[a] -= AW[e]; c[b] += AW[e]; }
    for (int e = ABEG[v], ee = ABEG[v + 1]; e < ee; e++) { int *c = CNT[ATO[e]]; c[b] -= AW[e]; c[a] += AW[e]; }
    s.grp[u] = (int8_t)b; s.grp[v] = (int8_t)a;
}

void read_input() {
    if (scanf("%d %d %d", &N, &K, &E) == 3 && N >= 2) {
        N = min(N, MAXN); K = min(K, MAXK); E = min(E, MAXE);
        for (int e = 0; e < E; e++) scanf("%d %d %d", &EU[e], &EV[e], &EW[e]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 201);
        N = 600; K = 6; E = 6000;
        static int cl[MAXN];
        for (int i = 0; i < N; i++) cl[i] = (int)g.next(6);
        for (int e = 0; e < E; e++) {
            int u, v;
            while (true) {
                if (g.nextf() < 0.7f) {
                    const int c = (int)g.next(6);
                    static int lst[MAXN]; int m = 0;
                    for (int i = 0; i < N; i++) if (cl[i] == c) lst[m++] = i;
                    if (m < 2) continue;
                    u = lst[g.next((uint32_t)m)]; v = lst[g.next((uint32_t)m)];
                } else { u = (int)g.next((uint32_t)N); v = (int)g.next((uint32_t)N); }
                if (u != v) break;
            }
            EU[e] = u; EV[e] = v; EW[e] = 1 + (int)g.next(10);
        }
    }
    // ---- 多重辺をまとめた CSR 隣接リストと隣接ビットセットを作る ----
    static int cntdeg[MAXN + 1];
    memset(cntdeg, 0, sizeof(int) * (size_t)(N + 1));
    for (int e = 0; e < E; e++) { cntdeg[EU[e]]++; cntdeg[EV[e]]++; }
    ABEG[0] = 0;
    for (int i = 0; i < N; i++) ABEG[i + 1] = ABEG[i] + cntdeg[i];
    static int pos[MAXN];
    for (int i = 0; i < N; i++) pos[i] = ABEG[i];
    static int tmpto[2 * MAXE], tmpw[2 * MAXE];
    for (int e = 0; e < E; e++) {
        tmpto[pos[EU[e]]] = EV[e]; tmpw[pos[EU[e]]++] = EW[e];
        tmpto[pos[EV[e]]] = EU[e]; tmpw[pos[EV[e]]++] = EW[e];
    }
    memset(ABIT, 0, sizeof(uint64_t) * (size_t)N * WBIT);
    int w = 0;
    static int ord[2 * MAXE];
    for (int i = 0; i < N; i++) {
        const int b = ABEG[i], en = ABEG[i + 1];
        int m = 0;
        for (int j = b; j < en; j++) ord[m++] = j;
        sort(ord, ord + m, [](int x, int y) { return tmpto[x] < tmpto[y]; });
        const int nb = w;
        for (int j = 0; j < m; j++) {
            const int t = tmpto[ord[j]];
            if (w > nb && ATO[w - 1] == t) AW[w - 1] += tmpw[ord[j]];   // 多重辺はまとめる
            else { ATO[w] = t; AW[w] = tmpw[ord[j]]; w++; }
            ABIT[i][(unsigned)t >> 6] |= 1ull << ((unsigned)t & 63);
        }
        cntdeg[i] = nb;                       // 一時的に開始位置を保存
    }
    for (int i = 0; i < N; i++) ABEG[i] = cntdeg[i];
    ABEG[N] = w;
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
