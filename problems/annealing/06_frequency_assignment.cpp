// =============================================================================
//  [焼きなまし系 06] 周波数割当 (無線基地局の干渉最小化)
// =============================================================================
//  【問題】
//    平面上に N 個の基地局があり、それぞれに K 種類の周波数のどれか 1 つを割り当てる。
//    2 局 i,j の距離が D 未満のとき、次の干渉ペナルティが発生する。
//      ・同じ周波数        : (D - dist)^2
//      ・周波数の差が 1    : (D - dist)^2 / 4
//      ・それ以外          : 0
//    総干渉ペナルティを最小化せよ。
//  【入力】
//    N K D
//    x_i y_i   (N 行)
//  【出力】
//    N 行。基地局 i に割り当てた周波数番号 (0..K-1)。
//  【スコア】 総干渉ペナルティ (小さいほど良い)。
//  【入力生成方法】
//    N=400, K=8, D=120 固定。x,y は [0,1000] の一様実数。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     同じ周波数だけでなく「隣の周波数」も (1/4 の重みで) 干渉するのがこの問題の厄介なところ。
//     普通のグラフ彩色の貪欲だと「同じ色を避ける」ことしか考えないので、
//     隣接周波数の干渉が積み上がって良い解にならない。
//     一方、解は「局 -> 周波数」の単純な割当なので、1 局の周波数を変える近傍で自由に動ける。
//     焼きなましがそのまま効く典型。
//
//   ● 状態 (State)
//     局 -> 周波数の割当 f[] と、
//     COST[局][周波数] = 「その局をその周波数にしたときの干渉合計」の表。
//     この表があると 1 局変更の差分が引き算 1 回で済む。
//
//   ● 手の作り方
//     確率 P_NB で「1 局の周波数を変える」、残りで「隣接する 2 局の周波数を入れ替える」。
//     ・変更する周波数は必ず今と別のものを提案する (同じものを引くと何も起きない)
//     ・入れ替えの相手は「隣接している局」に限る。遠い局と入れ替えても干渉は変わらないので無駄
//
//   ● 評価値
//     Σ (距離 D 未満のペアについて) 同じ周波数なら (D-距離)^2、差が 1 なら その 1/4、それ以外 0。
//     最小化。
//
//   ● 差分計算 / 高速化
//     1 局 v を a -> b に変える差分は COST[v][b] - COST[v][a] で O(1)。
//     採用したときだけ、v の隣接局の COST を O(deg) で更新する。
//     却下される手の方が圧倒的に多いので、この「却下は O(1)、採用時だけ O(deg)」が効く。
//     隣接リストは CSR (連続配列) にしてキャッシュに載せる。
//     これらで反復回数が 16.6M から 110M へ 6.6 倍になった。
//
//   ● つまずきポイント
//     ・COST 表の更新を忘れる/間違えると、差分だけ見ていても気付けない。
//       [verify] が 1 手ずつ全計算と突き合わせるので、ここで守られる。
//     ・入れ替え近傍で、2 局が隣接している場合の項を二重に数えないこと。
//
//   ● さらに伸ばすなら
//     ・時間を 9.5 倍にしてもスコアは 1.6% しか動かないので、速度より近傍の質を攻める方が良い
//     ・「干渉の大きい局を優先的に選ぶ」重み付きサンプリングは試したが差が無かった
//     ・貪欲に最良周波数を選ぶ近傍は逆効果 (+0.3〜0.6% 悪化)。決定的すぎて多様性を失う
//
//  【改善】
//    (1) 隣接リストを連結リストから CSR へ、(2) COST[局][周波数] 表を持って 1 局変更の差分を
//    O(deg) -> O(1) 化 (表の更新は採用時のみ)、(3) 入れ替え近傍の相手を「隣接局」に限定、
//    (4) 提案する周波数を必ず今と別のものにする、(5) 初期解を貪欲 2 巡で作る。
//    反復回数が 16.6M -> 110M (6.6 倍) に増え、近傍の当たりも良くなった。
//    seed 0..4 合計: 変更前 1915493 -> 変更後 1845499 (-3.7%)   ※seed 5..9 でも -3.7%
//
//  【採用したライブラリ】 simulated annealing/3_simulated_annealing
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=1222720 / 2_hill_climbing_kick=1161401 / 3_simulated_annealing=1140832 / 4_multi_start_annealing=1144734 / 5_iterated_annealing=1142840
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
float TEMP_START = 2000.0f;   // p1: 開始温度
float TEMP_END   = 5.0f;   // p2: 終了温度
float P_NB       = 0.7f;   // p3: 1 局の周波数を変える確率

void load_params() {
    pick_env("p1", TEMP_START);
    pick_env("p2", TEMP_END);
    pick_env("p3", P_NB);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (周波数割当)
// #############################################################################
constexpr int MAXN = 600, MAXK = 16, MAXADJ = 200000;
constexpr int CST = MAXK + 2;                 // コスト表 1 行の幅 (両端に番兵 1 つずつ)

int   N, K;
float DLIM;
// 隣接リストは CSR (連続配列) で持つ。ポインタ追跡が無いのでキャッシュに優しい。
static int   ADJ_S[MAXN + 1];
static int   ADJ_T[MAXADJ];
static float ADJ_W[MAXADJ];
// COST[v][k] = 局 v の周波数を k にしたときの「v が関わる干渉ペナルティの合計」
// これを持っておくと 1 局変更の差分が O(1) で求まる (更新は採用時だけ O(deg))。
static float COST[MAXN * CST];

struct State {
    float  score;
    int8_t f[MAXN];
};

struct Move { int i, j, to; };
static Move g_mv;

static inline float pen(float w, int a, int b) {
    const int d = abs(a - b);
    return (d == 0) ? w : (d == 1 ? w * 0.25f : 0.0f);
}

float full_score(const State &s) {
    float t = 0.0f;
    for (int i = 0; i < N; i++)
        for (int e = ADJ_S[i]; e < ADJ_S[i + 1]; e++)
            if (ADJ_T[e] > i) t += pen(ADJ_W[e], s.f[i], s.f[ADJ_T[e]]);
    return t;
}

// コスト表を全計算する
static void build_cost(const State &s) {
    for (int i = 0; i < N * CST; i++) COST[i] = 0.0f;
    for (int v = 0; v < N; v++) {
        float *c = COST + v * CST;
        for (int e = ADJ_S[v]; e < ADJ_S[v + 1]; e++) {
            const float w = ADJ_W[e];
            const int   g = s.f[ADJ_T[e]];      // 番兵があるので範囲チェック不要
            c[g] += 0.25f * w; c[g + 1] += w; c[g + 2] += 0.25f * w;
        }
    }
}

void init_state(State &s) {
    for (int i = 0; i < N; i++) s.f[i] = (int8_t)rng.next((uint32_t)K);
    build_cost(s);
    // 貪欲に 1 巡: 各局を「今いちばん安い周波数」へ動かしておく (初期解の底上げ)
    for (int pass = 0; pass < 2; pass++)
        for (int v = 0; v < N; v++) {
            const float *c = COST + v * CST;
            int   bk = s.f[v];
            float bv = c[bk + 1];
            for (int k = 0; k < K; k++) if (c[k + 1] < bv) { bv = c[k + 1]; bk = k; }
            if (bk == s.f[v]) continue;
            const int a = s.f[v];
            for (int e = ADJ_S[v]; e < ADJ_S[v + 1]; e++) {
                float *cc = COST + ADJ_T[e] * CST;
                const float w = ADJ_W[e], q = 0.25f * w;
                cc[a] -= q; cc[a + 1] -= w; cc[a + 2] -= q;
                cc[bk] += q; cc[bk + 1] += w; cc[bk + 2] += q;
            }
            s.f[v] = (int8_t)bk;
        }
    s.score = full_score(s);
}

inline void modify(State &s) {
    const int i = (int)rng.next((uint32_t)N);
    g_mv.i = i;
    if (rng.nextf() < P_NB) {
        g_mv.j = -1;
        // 今と違う周波数を必ず選ぶ (無駄な「変化なし」提案をしない)
        int b = (int)rng.next((uint32_t)(K - 1));
        if (b >= s.f[i]) b++;
        g_mv.to = b;
    } else {
        // 入れ替えは「同じ局の隣接局」との間だけにする (遠い 2 局を入れ替えても意味が薄い)
        const int st = ADJ_S[i], en = ADJ_S[i + 1];
        if (st == en) { g_mv.j = -1; int b = (int)rng.next((uint32_t)(K - 1)); if (b >= s.f[i]) b++; g_mv.to = b; return; }
        g_mv.j  = ADJ_T[st + (int)rng.next((uint32_t)(en - st))];
        g_mv.to = -1;
    }
}

inline float calc_score(State &s) {
    const int i = g_mv.i;
    const float *ci = COST + i * CST;
    if (g_mv.j < 0) {
        return ci[g_mv.to + 1] - ci[(int)s.f[i] + 1];
    }
    const int j = g_mv.j;
    const int a = s.f[i], b = s.f[j];
    if (a == b) return 0.0f;
    const float *cj = COST + j * CST;
    float d = (ci[b + 1] - ci[a + 1]) + (cj[a + 1] - cj[b + 1]);
    // i と j が隣接している場合、この 2 局間のペナルティは入れ替えても変わらないので補正する
    // (上の式では 2*w - 2*w*m(a,b) 余計に数えている)
    const int st = ADJ_S[i], en = ADJ_S[i + 1];
    for (int e = st; e < en; e++)
        if (ADJ_T[e] == j) { const float w = ADJ_W[e]; d -= 2.0f * (w - pen(w, a, b)); break; }
    return d;
}

// 局 v の周波数を a -> b に変えたときのコスト表の更新
static inline void upd_cost(int v, int a, int b) {
    const int st = ADJ_S[v], en = ADJ_S[v + 1];
    for (int e = st; e < en; e++) {
        float *c = COST + ADJ_T[e] * CST;
        const float w = ADJ_W[e], q = 0.25f * w;
        c[a] -= q; c[a + 1] -= w; c[a + 2] -= q;
        c[b] += q; c[b + 1] += w; c[b + 2] += q;
    }
}

inline void apply_move(State &s) {
    const int i = g_mv.i;
    if (g_mv.j < 0) {
        const int a = s.f[i], b = g_mv.to;
        upd_cost(i, a, b);
        s.f[i] = (int8_t)b;
    } else {
        const int j = g_mv.j;
        const int a = s.f[i], b = s.f[j];
        upd_cost(i, a, b);          // i: a -> b
        upd_cost(j, b, a);          // j: b -> a
        s.f[i] = (int8_t)b; s.f[j] = (int8_t)a;
    }
}

void read_input() {
    static float px[MAXN], py[MAXN];
    if (scanf("%d %d %f", &N, &K, &DLIM) == 3 && N >= 1) {
        N = min(N, MAXN); K = min(K, MAXK);
        for (int i = 0; i < N; i++) scanf("%f %f", &px[i], &py[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 206);
        N = 400; K = 8; DLIM = 120.0f;
        for (int i = 0; i < N; i++) { px[i] = g.nextf() * 1000.0f; py[i] = g.nextf() * 1000.0f; }
    }
    // ---- 距離 D 未満のペアを CSR 形式の隣接リストにする ----
    static int deg[MAXN];
    for (int i = 0; i < N; i++) deg[i] = 0;
    static int    eu[MAXADJ / 2], ev[MAXADJ / 2];
    static float  ew[MAXADJ / 2];
    int m = 0;
    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++) {
            const float dx = px[i] - px[j], dy = py[i] - py[j];
            const float d = sqrtf(dx * dx + dy * dy);
            if (d >= DLIM) continue;
            if (m + 1 > MAXADJ / 2) continue;
            eu[m] = i; ev[m] = j; ew[m] = (DLIM - d) * (DLIM - d); m++;
            deg[i]++; deg[j]++;
        }
    ADJ_S[0] = 0;
    for (int i = 0; i < N; i++) ADJ_S[i + 1] = ADJ_S[i] + deg[i];
    static int cur[MAXN];
    for (int i = 0; i < N; i++) cur[i] = ADJ_S[i];
    for (int e = 0; e < m; e++) {
        const int i = eu[e], j = ev[e]; const float w = ew[e];
        ADJ_T[cur[i]] = j; ADJ_W[cur[i]] = w; cur[i]++;
        ADJ_T[cur[j]] = i; ADJ_W[cur[j]] = w; cur[j]++;
    }
}

void output(const State &s) {
    string r; r.reserve((size_t)N * 3);
    for (int i = 0; i < N; i++) { r += to_string((int)s.f[i]); r += '\n'; }
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
