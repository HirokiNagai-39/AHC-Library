// =============================================================================
//  [モンテカルロ系 05] スリップするロボット (確率的な移動)
// =============================================================================
//  【問題】
//    H×W のグリッドに資源が置かれている (セル (i,j) の資源量 w_ij は事前に分かっている)。
//    ロボットは (0,0) から出発し、毎ターン上下左右のどれかを指示する。
//    ただし確率 SLIP で「指示と違う 3 方向のいずれか」に動いてしまう (どちらに滑るかは動いた後に分かる)。
//    壁の外へ動く指示・スリップの場合はその場に留まる。
//    移動先のセルの資源を初めて訪れたときだけ回収できる。T ターンでの回収量を最大化せよ。
//  【入力】
//    H W T SLIP(×10000)
//    w を H 行 W 列
//    s_t (0..9999) と r_t (0..2) を T 行  (ジャッジ側のスリップ判定。turn に到達するまで見てはいけない)
//  【出力】
//    T 行。各ターンの指示 "U"/"D"/"L"/"R"。
//  【スコア】 回収した資源量 (大きいほど良い)。
//  【入力生成方法】
//    H=W=10, T=120, SLIP=0.25 固定。w_ij は 0..20 の一様整数。
//    s_t は 0..9999 の一様整数、r_t は 0..2 の一様整数 (スリップ時に選ばれる方向のインデックス)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     移動が確率的なので、「決めた経路どおりに動く」前提のビームサーチは当てにならない。
//     各方向について未来のスリップ判定をランダムに 1 本引いて (シナリオ) 最後までシミュレートし、
//     平均回収量が最大の方向を選ぶ。
//     ★全候補が同じスリップ乱数列を使うので、運良く滑らなかった候補が勝つことがない。
//     この問題では**rollout を軽くすること**が最も効いた。
//
//   ● 状態 (State)
//     現在位置、訪問済みセルのビット、ターン、直前に指示した方向。
//
//   ● 手の作り方
//     上下左右の 4 方向。
//
//   ● 評価値 / rollout の方策
//     未訪問セルの価値を距離で割って足し合わせた**ポテンシャル場**
//       F[c] = Σ_{未訪問 j} w_j / (dist(c, j) + 0.5)^2
//     を作り、隣 4 マスのうち F が最大の方へ 1 歩進む。
//     「一番おいしい 1 セルへ向かう」貪欲と違い、資源のかたまりを狙えるのが利点。
//
//   ● 差分計算 / 高速化
//     ここが本題。ポテンシャル場はターン開始時に 1 回だけ作り (全候補で共通)、
//     セルを回収したらその 1 行を引くだけにする。こうすると rollout の 1 ターンが O(1) になる。
//     初版は毎ターン全 100 マスを走査していたので、シナリオ数が 338 本/ターンしかなかった。
//     これが 4557 本/ターンまで増えた (13 倍)。
//
//   ● つまずきポイント
//     ・カーネルの形 (分母の +0.5 や指数 2) は格子探索で確かめること。
//       平坦な最適域があるので、そこに入っていれば細かい値は問わない。
//     ・スリップ方向の決め方 (指示以外の 3 方向から等確率) は採点側と完全に一致させること。
//
//   ● さらに伸ばすなら
//     ・TIME_ALPHA を上げて序盤に時間を厚く配る案は -0.5% で不採用だった
//     ・伸ばすなら「数ターン先までの到達確率分布」を持って期待回収量を直接計算する方向
//
//  【改善】
//    rollout の方策を「一番おいしい 1 セルへ向かう貪欲 (毎ターン全マス走査)」から
//    ポテンシャル場 F[c]=Σ_未訪問 w_j/(dist+0.5)^2 の勾配を登る方策に変更した。
//    資源のかたまりを狙えるうえ、場はターン開始時に 1 回作って回収時に 1 行引くだけなので
//    1 ターン O(1) になり大幅に軽い (シナリオ数 338 -> 4557 本/ターン)。
//    seed 0..4 合計: 変更前 4663 -> 変更後 4750 (+1.9%)   [検証 seed 5..9: 4548 -> 4660 (+2.5%)]
//
//  【採用したライブラリ】 monte carlo/2_successive_halving
//    実測比較 (seed 0,1,2 の合計スコア): 1_monte_carlo=2775 / 2_successive_halving=2822
// =============================================================================
// =============================================================================
//  モンテカルロ法 + 逐次絞り込み (Successive Halving)   ---  AHC 用 高速テンプレート
// =============================================================================
//  フェーズごとに下位候補を切り捨て、有望な候補にシナリオを集中させる。同じ時間でより深く比較できる。
//
//  【この手法が向く問題】
//    未来が確率的に決まる問題 (何が来るか分からない / 相手の行動が分からない 等)。
//    「今この手を選んだら、最終的に平均でどれくらいのスコアになるか」を
//    ランダムな未来(シナリオ)を何本も試して見積もり、期待値が最良の手を選ぶ。
//
//  【★シナリオを公平にするための 4 原則★】  ← ここが最重要
//    1. 1 ラウンドで作ったシナリオは「全候補」で使い回す (共通乱数法)。
//       候補ごとに別のシナリオを引くと、運の良いシナリオを引いた候補が勝ってしまう。
//       同じ未来で比べると差だけが残るので、少ないシナリオ数でも正しく比較できる。
//    2. シナリオは「ターン番号で引ける配列」にする。乱数を消費順に引くと、
//       候補によって行動が変わった瞬間に未来そのものが変わってしまう。
//    3. rollout の中で方策がランダムに動く場合、その乱数も候補間で同じ種から始める。
//    4. 時間切れの打ち切りは必ず「ラウンド境界」で行う。ラウンドの途中で切ると
//       候補ごとにシナリオ数が変わり、比較が壊れる。
//    ※ 全候補のシナリオ数が必ず等しくなるので、平均を取らず「合計」のまま比較できる(除算不要)。
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct Move     { ... };                  ... 手
//    struct State    { float score; ... };     ... 状態 (score は必須)
//    struct Scenario { ... };                  ... 未来の不確定要素 1 本ぶん
//    init_state(State&)                        ... 初期状態
//    enum_moves(const State&, Move*)           ... 今の候補手を全列挙して個数を返す
//    calc_score(const State&, Move)            ... その手で確定するスコア差分
//    apply_move(State&, Move)                  ... 手を適用する (score は触らない)
//    reveal_turn(State&, int t)                ... ターン t で初めて分かる情報を state に入れる
//    gen_scenario(Scenario&, Xor128&, int)     ... シナリオを 1 本作る
//    rollout(State&, const Scenario&, Xor128&) ... 最後までシミュレートして最終スコアを返す
//
//  【高速化のポイント】
//    ・シナリオは 1 ラウンドに 1 本だけ作り、全候補で共有する (生成コストを候補数で割れる)
//    ・候補を適用した直後の状態を 1 回だけ作り置きし、rollout ごとにそこからコピーする
//    ・シナリオ数は時間から自動決定。1 ラウンドの実測時間で「次が入るか」を判定して打ち切る
//    ・期待値の集計は double (float だと数万回の加算で精度が落ちる)
//    ・時間計測は rdtsc/cntvct 直読み、1 ラウンドに 1 回だけ
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
float KER_A       = 0.5f;  // p1: ポテンシャル場の距離オフセット
float KER_B       = 2.0f;  // p4: ポテンシャル場の距離減衰の指数
float TIME_ALPHA  = 0.0f;  // p2: ターンごとの時間配分 (0=均等, 大きいほど序盤に厚く)
int   HALVE_RATE  = 2;   // p3: 1 フェーズごとに候補を 1/HALVE_RATE に絞る
int   MAX_SCENARIO = 1 << 30;

void load_params() {
    pick_env("p1", KER_A);
    pick_env("p4", KER_B);
    pick_env("p2", TIME_ALPHA);
    pick_env("p3", HALVE_RATE);
    pick_env("MAX_SCENARIO", MAX_SCENARIO);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化
// constexpr bool MAXIMIZE = false;    // ← スコア最小化

inline bool is_better(double a, double b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }
constexpr double WORST_SCORE = MAXIMIZE ? -1e300 : 1e300;

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS = 1900.0f;   // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr float ROUND_MARGIN  = 1.10f;     // 「次のラウンドが入るか」の判定に掛ける安全係数
// (この手法は探索の設定より「rollout をどれだけ軽くできるか」が効くので、ここは少なめ)

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_rounds = 0, g_rollouts = 0;
static int g_min_sc = INT32_MAX, g_max_sc = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (スリップするロボット)
// #############################################################################
constexpr int MAXH = 16, MAXW = 16, MAXC = MAXH * MAXW, MAXT = 300;
constexpr int MAX_CAND = 4;

int H, W, T, SLIP;
static int WT[MAXC];
static int TRUE_S[MAXT], TRUE_R[MAXT];
static const int DX[4] = {-1, 1, 0, 0}, DY[4] = {0, 0, -1, 1};
static int   HW;
static int   NB[MAXC][4];        // 隣接セル (壁の外は -1)
static float KER[MAXC][MAXC];    // KER[i][j] = 1/(マンハッタン距離+KER_A)^KER_B

struct Move { int8_t d; };

struct State {
    float    score;
    int32_t  r, c;
    uint64_t vis[4];
    int32_t  turn;
    int8_t   lastd;
};

static inline int actual_dir(int d, int s, int rr) {
    if (s >= SLIP) return d;
    int cnt = 0;
    for (int k = 0; k < 4; k++) if (k != d) { if (cnt == rr) return k; cnt++; }
    return d;
}

inline void reveal_turn(State &s, int turn) {
    if (turn > 0) {
        const int d = actual_dir(s.lastd, TRUE_S[turn - 1], TRUE_R[turn - 1]);
        const int nr = s.r + DX[d], nc = s.c + DY[d];
        if (nr >= 0 && nr < H && nc >= 0 && nc < W) {
            s.r = nr; s.c = nc;
            const int id = nr * W + nc;
            if (!((s.vis[id >> 6] >> (id & 63)) & 1)) { s.vis[id >> 6] |= 1ULL << (id & 63); s.score += (float)WT[id]; }
        }
    }
    s.turn = turn;
}

void init_state(State &s) {
    s.score = (float)WT[0]; s.r = 0; s.c = 0; s.turn = 0; s.lastd = 0;
    memset(s.vis, 0, sizeof(s.vis));
    s.vis[0] = 1;
    HW = H * W;
    for (int i = 0; i < HW; i++) {
        const int r = i / W, c = i % W;
        for (int d = 0; d < 4; d++) {
            const int nr = r + DX[d], nc = c + DY[d];
            NB[i][d] = (nr >= 0 && nr < H && nc >= 0 && nc < W) ? (nr * W + nc) : -1;
        }
        for (int j = 0; j < HW; j++) {
            const float dd = (float)(abs(r - j / W) + abs(c - j % W));
            KER[i][j] = 1.0f / powf(dd + KER_A, KER_B);
        }
    }
}

inline int enum_moves(const State &s, Move *out) { (void)s; for (int d = 0; d < 4; d++) out[d].d = (int8_t)d; return 4; }
inline float calc_score(const State &s, const Move &mv) { (void)s; (void)mv; return 0.0f; }
inline void apply_move(State &s, const Move &mv) { s.lastd = mv.d; }

struct Scenario { int16_t s[MAXT]; int8_t r[MAXT]; };
inline void gen_scenario(Scenario &sc, Xor128 &rnd, int from) {
    for (int t = max(0, from - 1); t < T; t++) { sc.s[t] = (int16_t)rnd.next(10000); sc.r[t] = (int8_t)rnd.next(3); }
}

// ---- rollout: ポテンシャル場による方策 -----------------------------------
//  「1 つの目標セルへ向かう」貪欲だと、まとまった資源のかたまりを見落とすうえ
//  毎ターン全マス走査が必要で重い。そこで
//      F[cell] = Σ_{未訪問 j} w_j / (dist(cell,j)+KER_A)^KER_B
//  という場を持ち、隣 4 マスのうち F が最大の方へ進む (1 ターン O(1))。
//  場はセルを回収したときに 1 行ぶんの引き算で更新できる (SIMD で流せる)。
//  ターン開始時の場は全候補で共通なので 1 ターンに 1 回だけ作れば良い。
static int      g_ft = -1;
static uint64_t g_fv = ~0ULL;
static float    g_field0[MAXC];

inline float rollout(State &s, const Scenario &sc, Xor128 &rnd) {
    (void)rnd;
    if (g_ft != s.turn || g_fv != s.vis[0]) {          // ★ターンに 1 回だけ場を作る
        for (int i = 0; i < HW; i++) g_field0[i] = 0.0f;
        for (int j = 0; j < HW; j++) {
            if ((s.vis[j >> 6] >> (j & 63)) & 1) continue;
            const float w = (float)WT[j];
            if (w == 0.0f) continue;
            const float *g = KER[j];
            for (int i = 0; i < HW; i++) g_field0[i] += w * g[i];
        }
        g_ft = s.turn; g_fv = s.vis[0];
    }
    static float F[MAXC];
    memcpy(F, g_field0, sizeof(float) * HW);

    float total = s.score;
    uint64_t vis[4]; memcpy(vis, s.vis, sizeof(vis));
    int r = s.r, c = s.c, want = s.lastd;
    for (int t = s.turn; t < T; t++) {
        if (t > s.turn) {
            const int *nb = NB[r * W + c];
            float bv = -1e30f; want = 0;
            for (int d = 0; d < 4; d++) {
                const int j = nb[d];
                if (j >= 0 && F[j] > bv) { bv = F[j]; want = d; }
            }
        }
        const int d = actual_dir(want, sc.s[t], sc.r[t]);
        const int nr = r + DX[d], nc = c + DY[d];
        if (nr >= 0 && nr < H && nc >= 0 && nc < W) {
            r = nr; c = nc;
            const int id = nr * W + nc;
            if (!((vis[id >> 6] >> (id & 63)) & 1)) {
                vis[id >> 6] |= 1ULL << (id & 63);
                const float w = (float)WT[id];
                total += w;
                if (w != 0.0f) {
                    const float *g = KER[id];
                    for (int i = 0; i < HW; i++) F[i] -= w * g[i];
                }
            }
        }
    }
    return total;
}

void read_input() {
    if (scanf("%d %d %d %d", &H, &W, &T, &SLIP) == 4 && H >= 2) {
        H = min(H, MAXH); W = min(W, MAXW); T = min(T, MAXT);
        for (int i = 0; i < H * W; i++) scanf("%d", &WT[i]);
        for (int t = 0; t < T; t++) scanf("%d %d", &TRUE_S[t], &TRUE_R[t]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 405);
        H = 10; W = 10; T = 120; SLIP = 2500;
        for (int i = 0; i < H * W; i++) WT[i] = (int)g.next(21);
        for (int t = 0; t < T; t++) { TRUE_S[t] = (int)g.next(10000); TRUE_R[t] = (int)g.next(3); }
    }
}

void output(const vector<Move> &hist) {
    static const char CH[4] = {'U', 'D', 'L', 'R'};
    string r; r.reserve(hist.size() * 2);
    for (const Move &mv : hist) { r += CH[mv.d]; r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &hist) {
    uint64_t vis[4] = {0, 0, 0, 0};
    int r = 0, c = 0;
    vis[0] = 1;
    float total = (float)WT[0];
    for (int t = 0; t < T && t < (int)hist.size(); t++) {
        const int d = actual_dir(hist[t].d, TRUE_S[t], TRUE_R[t]);
        const int nr = r + DX[d], nc = c + DY[d];
        if (nr < 0 || nr >= H || nc < 0 || nc >= W) continue;
        r = nr; c = nc;
        const int id = nr * W + nc;
        if (!((vis[id >> 6] >> (id & 63)) & 1)) { vis[id >> 6] |= 1ULL << (id & 63); total += (float)WT[id]; }
    }
    return total;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
//  ターンごとの時間配分
// =============================================================================
//  TIME_ALPHA = 0 なら全ターン均等。大きくすると序盤に多く時間を割り当てる。
//  各ターンの締切は「累積」で持つので、早く終わったターンの余りは自動的に後ろへ回る。
static float g_deadline[MAXT + 1];
static void plan_time(float begin_ms, float total_ms) {
    double sum = 0.0;
    for (int t = 0; t < T; t++) sum += pow((double)(T - t) / (double)T, (double)TIME_ALPHA);
    double acc = 0.0;
    for (int t = 0; t < T; t++) {
        acc += pow((double)(T - t) / (double)T, (double)TIME_ALPHA);
        g_deadline[t] = begin_ms + total_ms * (float)(acc / sum);
    }
}

// ---- 1 ターンぶんの作業領域 (静的確保。探索中に malloc しない) ----
static Move     g_cand[MAX_CAND];      // 候補手
static State    g_base[MAX_CAND];      // 候補を適用した直後の状態 (作り置き)
static double   g_sum [MAX_CAND];      // 候補ごとのスコア合計 (float だと精度が落ちる)
static State    g_work;                // rollout 用の作業状態
static Scenario g_sc;                  // ★シナリオは 1 本だけ持ち、全候補で共有する
static uint32_t g_round_id = 0;        // シナリオの種 (ターンをまたいで別の未来を引く)

// =============================================================================
// 7. モンテカルロ法 + 逐次絞り込み (Successive Halving) 本体
// =============================================================================
//  弱い候補にシナリオを使い続けるのは無駄なので、フェーズごとに下位を切り捨てて
//  生き残った候補にシナリオを集中させる。同じ時間でより多くのシナリオを有望手に回せる。
//
//  ★公平性: 1 つのフェーズの中では「生き残っている全候補」が必ず同じシナリオを試す。
//    生き残り候補どうしは常に同じシナリオ集合で比較されるので、比較は壊れない。
//    (途中で落ちた候補はシナリオ数が少ないが、落とす時点では全員同数で比べている)
Move choose_move(const State &state, float begin_ms, float deadline_ms) {
    const int C = enum_moves(state, g_cand);
    if (C <= 1) { g_min_sc = 0; return g_cand[0]; }

    for (int c = 0; c < C; c++) {
        g_base[c] = state;
        g_base[c].score += calc_score(state, g_cand[c]);
        apply_move(g_base[c], g_cand[c]);
        g_sum[c] = 0.0;
    }

    // 生存候補の添字リスト
    static int32_t alive_id[MAX_CAND];
    for (int c = 0; c < C; c++) alive_id[c] = c;
    int alive = C;

    // 何フェーズに分けるか (毎フェーズで 1/HALVE_RATE に絞る)
    int phases = 1;
    for (int c = C; c > 1;) { c = max(1, (c + HALVE_RATE - 1) / HALVE_RATE); phases++; }

    int   n     = 0;
    float prev  = timer.ms();
    float round = 0.0f;
    const float span = deadline_ms - begin_ms;

    for (int ph = 0; ph < phases; ph++) {
        const float ph_deadline = begin_ms + span * (float)(ph + 1) / (float)phases;
        // --- このフェーズのシナリオを回す ---
        while (n < MAX_SCENARIO) {
            if (n > 0 && prev + round * ROUND_MARGIN > ph_deadline) break;
            Xor128 rs; rs.seed(g_round_id++);
            gen_scenario(g_sc, rs, state.turn + 1);
            Xor128 rp; rp.seed(0x9E3779B9u ^ g_round_id);
            for (int i = 0; i < alive; i++) {           // ★生存候補は全員同じシナリオ
                const int c = alive_id[i];
                g_work = g_base[c];
                Xor128 r = rp;
                g_sum[c] += (double)rollout(g_work, g_sc, r);
            }
            n++; g_rollouts += alive;
            const float now = timer.ms();
            round = now - prev;
            prev  = now;
        }
        if (alive <= 1) break;
        // --- 上位だけ残す (全ソートせず nth_element) ---
        const int keep = max(1, (alive + HALVE_RATE - 1) / HALVE_RATE);
        nth_element(alive_id, alive_id + keep, alive_id + alive,
                    [](int32_t a, int32_t b) { return is_better(g_sum[a], g_sum[b]); });
        alive = keep;
        if (prev >= deadline_ms) break;
    }
    g_rounds += n;
    g_min_sc = min(g_min_sc, n); g_max_sc = max(g_max_sc, n);

    int best = alive_id[0];
    for (int i = 1; i < alive; i++) if (is_better(g_sum[alive_id[i]], g_sum[best])) best = alive_id[i];
    return g_cand[best];
}

// =============================================================================
// 8. main
// =============================================================================
int main() {
    timer.start();      // ★ 一番最初にタイマー開始
    load_params();      // 環境変数からパラメータを読む (optuna 用)
    read_input();

    static State state;
    init_state(state);
    plan_time(timer.ms(), TIME_LIMIT_MS - timer.ms());   // ターンごとの締切を決める

    vector<Move> hist(T);
    for (int t = 0; t < T; t++) {
        reveal_turn(state, t);              // ★このターンに初めて分かる情報を state に取り込む
        const Move mv = choose_move(state, t == 0 ? timer.ms() : g_deadline[t - 1], g_deadline[t]);
        hist[t] = mv;
        state.score += calc_score(state, mv);
        apply_move(state, mv);
    }
    // ★最終ターンの実現値を state に反映する。これが無いと下の [check] の running 側が
    //   最後の 1 ターンぶんだけ足りない値になり、差分計算のバグと区別できなくなる。
    if (T < MAXT) reveal_turn(state, T);
    output(hist);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)replay_true_score(hist));
    fprintf(stderr, "rounds = %lld, rollouts = %lld, scenarios/turn = %d..%d, time = %.1f ms\n",
            g_rounds, g_rollouts, g_min_sc == INT32_MAX ? 0 : g_min_sc, g_max_sc, (double)timer.ms());
    // ★実装のバグ検出: 進行中に積み上げたスコアと、出力を再生した真のスコアを比べる
    //   ※ 最終ターンの実現値は「次の reveal_turn()」で反映される作りなので、running 側は
    //      最後の 1 ターンぶんだけ足りない値になる。これは仕様であってバグではない。
    //      提出スコアは replay 側 (上の Score = ...) を見ること。それ以上にずれていたら
    //      calc_score / apply_move / reveal_turn のどれかが間違っている
    fprintf(stderr, "[check] running = %.0f / replay = %.0f\n",
            (double)state.score, (double)replay_true_score(hist));
    return 0;
}
