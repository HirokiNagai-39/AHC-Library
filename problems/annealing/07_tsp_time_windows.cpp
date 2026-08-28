// =============================================================================
//  [焼きなまし系 07] 時間枠付き巡回 (TSP with time windows)
// =============================================================================
//  【問題】
//    平面上に N 個の地点があり、地点 0 が出発地。地点 i には時間枠 [a_i, b_i] がある。
//    速度 1 で移動し (移動時間 = ユークリッド距離)、時刻 0 に地点 0 を出発して
//    地点 1..N-1 をすべて 1 回ずつ訪問し、地点 0 に戻る。
//    地点 i に時刻 t で着いたとき、t < a_i なら a_i まで待つ (出発は a_i)。t > b_i なら遅延 (t - b_i)。
//    コスト = 総移動距離 + 100 * (遅延の合計) を最小化せよ。
//  【入力】
//    N
//    x_i y_i a_i b_i   (N 行。地点 0 の a,b は 0 と十分大きい値)
//  【出力】
//    1 行に N-1 個の整数。地点 0 の次から訪問する順に地点番号を並べる。
//  【スコア】 上記コスト (小さいほど良い)。順列でなければ 0 点。
//  【入力生成方法】
//    N=100 固定。x,y は [0,1000] の一様実数。
//    a_i は [0,8000] の一様実数、b_i = a_i + (300..1500 の一様実数)。
//    時間枠は互いに独立なので、遅延ゼロの巡回路は普通存在しない。
//    「距離を縮める」ことと「遅延を減らす」ことのトレードオフを解くことになる。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     時間枠が付くと「ある地点の到着時刻がずれると、その後ろ全部がずれる」ので、
//     距離だけの TSP と違って O(1) の差分計算ができない。
//     素直にやると 1 手 O(N) の全再計算になるが、N=100 なら毎秒数百万回まわるので実は十分戦える。
//     「差分が取れないなら全計算でよい」という判断も大事。
//     そのうえで、後ろ半分を賢く打ち切れば 2 倍近く稼げる。
//
//   ● 状態 (State)
//     訪問順 ord[] に加えて、出発からの到着時刻 TT[] と累積コスト CC[]。
//     この 2 本があると「変更した区間より前」は計算不要になる。
//
//   ● 手の作り方
//     確率 P_2OPT で 2-opt (区間反転)、残りで or-opt (1 地点を別の位置へ移動)。
//     どちらも順列を保つので実行可能性は常に満たされる。
//
//   ● 評価値
//     総移動距離 + 100 × (遅延の合計)。最小化。
//     早着したら a_i まで待つので、到着時刻は max(到着, a_i) で更新していく。
//     この「待ち」があるおかげで、後ろの到着時刻がしばしば元に戻る (下記の打ち切りが効く理由)。
//
//   ● 差分計算 / 高速化
//     変更区間より前は TT[]/CC[] をそのまま使う。後ろは訪問順が変わらないので、
//     次のどちらかでたいてい途中で切り上げられる。
//       ・到着時刻が元と完全に一致した → 以降のコストも一致するのでそこで打ち切り
//       ・そこから先しばらく「全部遅刻していて待ちが無い」→ 到着時刻のずれ dt に対し
//         残りのコストは 100 × (地点数) × dt だけ動くので、その区間を丸ごと飛ばせる
//         (接尾辞の NXT[]/RMIN[] を前計算しておけば O(1) で判定できる)
//     これで反復数が 11M から 20M に増えた。
//
//   ● つまずきポイント
//     ・打ち切り判定に「直前の地点が変わっているか」を入れ忘れると、
//       区間の直前の地点が変わったケースで誤った打ち切りをする。
//       しかも apply_move 側で厳密値を入れ直していたため [check] でも [verify] でも検出できず、
//       A/B 比較が -1.5% 悪化して初めて気付いた。**検証機構は万能ではない**という実例。
//     ・この問題は同じ設定でも seed ごとに ±10% ぶれる。A/B は 20 seed で取ること。
//
//   ● さらに伸ばすなら
//     ・or-opt の区間長を伸ばす / swap 近傍を足す / 初期解を貪欲や締切順にする /
//       締切に間に合う位置を狙って挿入する、はいずれも A/B で悪化した (+4〜7%)
//     ・伸ばすなら「遅刻している地点の前後だけを狙う」近傍や、
//       遅延ペナルティを段階的に上げていく手法 (penalty ramping) の方向
//
//  【改善】
//    TT[]/CC[] の前計算と、後半の「打ち切り・区間スキップ」で差分計算を高速化 (反復数 11M -> 20M)。
//    近傍の種類と確率は元のまま (or-opt の区間長を伸ばす / swap を足す / 初期解を貪欲や締切順にする /
//    締切に間に合う位置を狙って挿入する、はいずれも A/B で悪化したので不採用)。
//    seed 0..4 合計: 変更前 72692939 -> 変更後 71097047 (-2.2%)
//    ※ この問題は同じ設定でも seed ごとに ±10% ぶれるので、seed 0..19 の 20 個で確認して -2.6%。
//
//  【採用したライブラリ】 simulated annealing/2_hill_climbing_kick
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=57033810 / 2_hill_climbing_kick=45661567 / 3_simulated_annealing=50939412 / 4_multi_start_annealing=51942896 / 5_iterated_annealing=51579522
// =============================================================================
// =============================================================================
//  キックあり山登り (Iterated Local Search)   ---  AHC 用 高速テンプレート
// =============================================================================
//  山登りが局所解で止まったら「キック」で解を大きく動かし、最良解から探索し直す。
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
float P_2OPT           = 0.60f;  // p1: 2-opt (区間反転) を選ぶ確率
float P_ORO            = 0.40f;  // p2: or-opt (区間移動) を選ぶ確率 (残りは swap)
int   SEG_MAX          = 999;    // p3: 2-opt で反転する区間の最大長
int   MOVE_MAX         = 999;    // p4: or-opt / swap の最大移動距離
int   ORO_MAXLEN       = 1;      // p5: or-opt で動かす区間の最大長
int   INIT_MODE        = 0;      // p6: 初期解 0=ランダム 1=最近傍 2=締切順 3=距離+遅延貪欲 4=距離+余裕貪欲
float INIT_W           = 0.3f;   // p7: INIT_MODE=4 の締切の重み
int   KICK_STRENGTH    = 3;      // p8: キック 1 回で無条件に適用するランダム遷移の回数
int   STAGNATION_LIMIT = 10000;  // p9: 改善が無い回数がこれを超えたらキックする

void load_params() {
    pick_env("p1", P_2OPT);
    pick_env("p2", P_ORO);
    pick_env("p3", SEG_MAX);
    pick_env("p4", MOVE_MAX);
    pick_env("p5", ORO_MAXLEN);
    pick_env("p6", INIT_MODE);
    pick_env("p7", INIT_W);
    pick_env("p8", KICK_STRENGTH);
    pick_env("p9", STAGNATION_LIMIT);
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

// ---- 山登り用 ----
constexpr bool  ACCEPT_EQUAL = true;    // 同スコア(横ばい)の遷移も採用する → 平坦な地形を抜けやすい
constexpr float EQ_EPS       = 1e-6f;   // 「同スコア」とみなす誤差 (スコアの大きさに応じて調整)
constexpr float ACCEPT_TH    = ACCEPT_EQUAL ? -EQ_EPS : 0.0f;   // gain > ACCEPT_TH なら採用

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
static ll g_kick = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (時間枠付き巡回)
// #############################################################################
constexpr int MAXN = 256;
constexpr float LATE_PEN = 100.0f;

int N, M;                       // M = N-1 (地点 0 以外の数)
static float PX[MAXN], PY[MAXN], TA[MAXN], TB[MAXN];
static float D[MAXN * MAXN];
static inline float dist(int i, int j) { return D[i * N + j]; }

struct State {
    float    score;
    uint32_t stamp;         // 下の前計算テーブルがどの状態のものかを示す通し番号
    int16_t  ord[MAXN];     // ord[0..N-2] : 地点 0 の次からの訪問順
};

// ---- 現在解に対する前計算テーブル (差分計算用) ----
//   TT[k] : 位置 k-1 を出発する時刻 (TT[0]=0)
//   CC[k] : 位置 0..k-1 で確定したコスト (移動距離 + 遅延ペナルティ)
//   VV[j] : 位置 j の「遅れ具合」。待ちが発生した位置は -INF (時刻をずらしても線形にならない印)
//   NXT[k]: k 以降で最初に VV<=0 になる位置。 RMIN[k]: [k, NXT[k]) の VV の最小値
static float TT[MAXN + 1], CC[MAXN + 1], VV[MAXN + 1], RMIN[MAXN + 1];
static int   NXT[MAXN + 1];
static uint32_t g_tab_stamp = 0xffffffffu, g_stamp_gen = 0;

// 位置 lo 以降の TT/CC/VV を作り直し、LM を張り直す (TT[lo],CC[lo] は正しい前提)
static inline void rebuild_from(const int16_t *ord, int lo) {
    float t = TT[lo], c = CC[lo];
    int prev = lo ? (int)ord[lo - 1] : 0;
    for (int k = lo; k < M; k++) {
        const int nd = (int)ord[k];
        const float d = D[prev * N + nd];
        c += d; t += d;
        if (t < TA[nd]) { t = TA[nd]; VV[k] = -1e30f; }   // 待ちが発生 → 線形近似の対象外
        else            { VV[k] = t - TB[nd]; }
        if (t > TB[nd]) c += LATE_PEN * (t - TB[nd]);
        TT[k + 1] = t; CC[k + 1] = c;
        prev = nd;
    }
    NXT[M] = M; RMIN[M] = 1e30f;
    for (int k = M - 1; k >= 0; k--) {
        if (VV[k] > 0.0f) { NXT[k] = NXT[k + 1]; RMIN[k] = min(VV[k], RMIN[k + 1]); }
        else              { NXT[k] = k;          RMIN[k] = 1e30f; }
    }
}
static inline float total_of_tables(const int16_t *ord) { return CC[M] + D[(int)ord[M - 1] * N]; }

// 訪問順 ord からコストを計算する (スコアの定義そのもの)
static inline float route_cost(const int16_t *ord) {
    float t = 0.0f, cost = 0.0f;
    int prev = 0;
    for (int k = 0; k < N - 1; k++) {
        const int c = ord[k];
        const float d = dist(prev, c);
        cost += d; t += d;
        if (t < TA[c]) t = TA[c];                 // 早着は待つ
        if (t > TB[c]) { cost += LATE_PEN * (t - TB[c]); }
        prev = c;
    }
    cost += dist(prev, 0);
    return cost;
}

float full_score(const State &s) { return route_cost(s.ord); }

// テーブルが s のものでなければ張り直す (ライブラリが s = best と巻き戻しても安全)
static inline void ensure_tables(State &s) {
    if (s.stamp != g_tab_stamp) {
        TT[0] = 0.0f; CC[0] = 0.0f;
        rebuild_from(s.ord, 0);
        g_tab_stamp = s.stamp;
    }
}

void init_state(State &s) {
    if (INIT_MODE == 0) {                        // ランダム順
        for (int i = 0; i < M; i++) s.ord[i] = (int16_t)(i + 1);
        for (int i = M - 1; i > 0; i--) swap(s.ord[i], s.ord[rng.next(i + 1)]);
    } else if (INIT_MODE == 2) {                 // 締切 TB の昇順
        static int idx[MAXN];
        for (int i = 0; i < M; i++) idx[i] = i + 1;
        sort(idx, idx + M, [](int a2, int b2) { return TB[a2] < TB[b2]; });
        for (int i = 0; i < M; i++) s.ord[i] = (int16_t)idx[i];
    } else {                                     // 貪欲構築 (O(N^2))
        static bool used[MAXN];
        for (int i = 0; i < N; i++) used[i] = false;
        float t = 0.0f; int prev = 0;
        for (int k = 0; k < M; k++) {
            int best = -1; float bestv = 0.0f;
            const float *Dp = D + prev * N;
            for (int c = 1; c < N; c++) {
                if (used[c]) continue;
                const float d = Dp[c];
                float nt = t + d; if (nt < TA[c]) nt = TA[c];
                float v;
                if (INIT_MODE == 1)      v = d;                                      // 最近傍
                else if (INIT_MODE == 3) v = d + (nt > TB[c] ? LATE_PEN * (nt - TB[c]) : 0.0f);
                else                     v = d + INIT_W * (TB[c] - nt);              // 距離 + 締切の余裕
                if (best < 0 || v < bestv) { best = c; bestv = v; }
            }
            used[best] = true;
            const float d = Dp[best];
            t += d; if (t < TA[best]) t = TA[best];
            s.ord[k] = (int16_t)best;
            prev = best;
        }
    }
    s.stamp = ++g_stamp_gen;
    TT[0] = 0.0f; CC[0] = 0.0f;
    rebuild_from(s.ord, 0);
    g_tab_stamp = s.stamp;
    s.score = total_of_tables(s.ord);
}

// ---- 近傍: すべて「位置 lo..lo+len-1 を g_seg[0..len-1] に差し替える」形に統一する ----
struct Move { int lo, len; };
static Move g_mv;
static int16_t g_seg[MAXN];
static float g_diff = 0.0f;

inline void modify(State &s) {
    ensure_tables(s);
    const float r = rng.nextf();
    if (r < P_2OPT) {                            // 2-opt: 区間 [i,j] を反転 (長さ <= SEG_MAX)
        const int i = (int)rng.next((uint32_t)(M - 1));
        const int jmax = min(M - 1, i + SEG_MAX - 1);
        const int j = i + 1 + (int)rng.next((uint32_t)(jmax - i));
        const int L = j - i + 1;
        for (int q = 0; q < L; q++) g_seg[q] = s.ord[j - q];
        g_mv.lo = i; g_mv.len = L;
    } else if (r < P_2OPT + P_ORO) {             // or-opt: 長さ L の区間を距離 <= MOVE_MAX 先へ
        int L = 1 + (int)rng.next((uint32_t)ORO_MAXLEN);
        if (L > M - 1) L = M - 1;
        const int span = M - L + 1;              // 区間の開始位置の範囲 [0, span-1]
        const int p = (int)rng.next((uint32_t)span);
        int d = 1 + (int)rng.next((uint32_t)min(MOVE_MAX, span - 1));
        if (rng.next(2)) d = -d;
        int q = p + d;                           // はみ出したら巻き戻す (無操作を作らない)
        if (q < 0) q += span; else if (q >= span) q -= span;
        if (q < p) {                             // [q..p+L-1] → ord[p..p+L-1] + ord[q..p-1]
            int u = 0;
            for (int v = 0; v < L; v++) g_seg[u++] = s.ord[p + v];
            for (int v = q; v < p; v++) g_seg[u++] = s.ord[v];
            g_mv.lo = q; g_mv.len = u;
        } else {                                 // [p..q+L-1] → ord[p+L..q+L-1] + ord[p..p+L-1]
            int u = 0;
            for (int v = p + L; v < q + L; v++) g_seg[u++] = s.ord[v];
            for (int v = 0; v < L; v++) g_seg[u++] = s.ord[p + v];
            g_mv.lo = p; g_mv.len = u;
        }
    } else {                                     // swap: 距離 <= MOVE_MAX の 2 地点を交換
        const int i = (int)rng.next((uint32_t)M);
        int d = 1 + (int)rng.next((uint32_t)min(MOVE_MAX, M - 1));
        if (rng.next(2)) d = -d;
        int j = i + d;
        if (j < 0) j += M; else if (j >= M) j -= M;
        const int a2 = min(i, j), b2 = max(i, j), L = b2 - a2 + 1;
        for (int q = 0; q < L; q++) g_seg[q] = s.ord[a2 + q];
        swap(g_seg[0], g_seg[L - 1]);
        g_mv.lo = a2; g_mv.len = L;
    }
}

// 差し替え後の総コスト - 現在のスコア。
//  差し替え区間より後ろは訪問順が変わらないので、
//   ・到着時刻が元と一致した瞬間 → 残りのコストも完全に一致するので即打ち切り。
//   ・一致しなくても「そこから m 手前まで全部遅刻していて待ちが無い」区間なら、
//     到着時刻のずれ dt に対し残りは LATE_PEN * (地点数) * dt だけ動くだけなので、
//     その区間を丸ごと飛ばせる (NXT / RMIN で O(1) 判定)。
//  ※ 差し替え区間の直後の 1 地点だけは「直前の地点」が変わっているので必ず歩く。
inline float calc_score(State &s) {
    const int lo = g_mv.lo, len = g_mv.len;
    float t = TT[lo], c = CC[lo];
    int prev = lo ? (int)s.ord[lo - 1] : 0;
    for (int q = 0; q < len; q++) {
        const int nd = (int)g_seg[q];
        const float d = D[prev * N + nd];
        c += d; t += d;
        if (t < TA[nd]) t = TA[nd];
        if (t > TB[nd]) c += LATE_PEN * (t - TB[nd]);
        prev = nd;
    }
    int k = lo + len;
    if (k < M) {                                    // 直前が変わっている 1 地点は必ず計算する
        const int nd = (int)s.ord[k];
        const float d = D[prev * N + nd];
        c += d; t += d;
        if (t < TA[nd]) t = TA[nd];
        if (t > TB[nd]) c += LATE_PEN * (t - TB[nd]);
        prev = nd; k++;
    }
    while (k < M) {
        const float dt = t - TT[k];
        if (dt == 0.0f) return (g_diff = c - CC[k]);
        const int m = NXT[k];
        if (m > k && RMIN[k] + (dt < 0.0f ? dt : 0.0f) > 0.0f) {
            c += (CC[m] - CC[k]) + LATE_PEN * (float)(m - k) * dt;
            t  = TT[m] + dt;
            prev = (int)s.ord[m - 1];
            k = m;
            continue;
        }
        const int nd = (int)s.ord[k];
        const float d = D[prev * N + nd];
        c += d; t += d;
        if (t < TA[nd]) t = TA[nd];
        if (t > TB[nd]) c += LATE_PEN * (t - TB[nd]);
        prev = nd; k++;
    }
    return (g_diff = c + D[prev * N] - s.score);
}

inline void apply_move(State &s) {
    const int lo = g_mv.lo, len = g_mv.len;
    memcpy(s.ord + lo, g_seg, sizeof(int16_t) * (size_t)len);
    rebuild_from(s.ord, lo);
    s.stamp = ++g_stamp_gen; g_tab_stamp = s.stamp;
    // 呼び出し側が直後に s.score += g_diff するので、厳密値 - 差分 を入れておく
    // (打ち切り近似を使っても score に誤差が溜まらない)
    s.score = total_of_tables(s.ord) - g_diff;
}

void read_input() {
    if (scanf("%d", &N) == 1 && N >= 3) {
        N = min(N, MAXN);
        for (int i = 0; i < N; i++) scanf("%f %f %f %f", &PX[i], &PY[i], &TA[i], &TB[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 207);
        N = 100;
        for (int i = 0; i < N; i++) { PX[i] = g.nextf() * 1000.0f; PY[i] = g.nextf() * 1000.0f; }
        TA[0] = 0.0f; TB[0] = 1e9f;
        for (int i = 1; i < N; i++) { TA[i] = g.nextf() * 8000.0f; TB[i] = TA[i] + 300.0f + g.nextf() * 1200.0f; }
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            const float dx = PX[i] - PX[j], dy = PY[i] - PY[j];
            D[i * N + j] = sqrtf(dx * dx + dy * dy);
        }
    M = N - 1;
}

void output(const State &s) {
    string r; r.reserve((size_t)N * 4);
    for (int i = 0; i < N - 1; i++) { if (i) r += ' '; r += to_string((int)s.ord[i]); }
    r += '\n';
    fputs(r.c_str(), stdout);
}
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
// 7. キックあり山登り (Iterated Local Search) 本体
// =============================================================================
//  山登りで局所解に落ちたら「キック」で解を強制的に大きく動かし、また山登りする。
//  キック前に必ず最良解へ戻すので、解が悪くなりっぱなしになることはない。
//  焼きなましの温度調整が難しい問題でも、パラメータが直感的で扱いやすい。

// キック: 近傍遷移を KICK_STRENGTH 回、採用判定なしで適用して解を大きく動かす
// (専用のキック近傍 ―― TSP なら double bridge など ―― を書くとより強力)
inline void kick(State &s) {
    for (int t = 0; t < KICK_STRENGTH; t++) {
        modify(s);
        const float d = calc_score(s);
        apply_move(s);
        s.score += d;                   // 差分を足していけばスコアの再計算は不要
    }
}

void hill_climbing_kick(State &s, float deadline_ms) {
    static State best;                  // 最良解 (キック前に必ずここへ戻る)
    best = s;
    int   stag  = 0;                    // 改善が無かった連続回数
    int   block = ITER_PER_CHECK;       // ★ 時間計測はこの回数に 1 回だけ
    float prev  = timer.ms();
    while (true) {
        // ---- 内側ループ: 時間計測を一切しない ----
        for (int it = 0; it < block; it++) {
            modify(s);
            const float diff = calc_score(s);
            const float gain = GAIN_SIGN * diff;
            if (gain > ACCEPT_TH) {
                apply_move(s);
                s.score += diff;
                g_accept++;
                if (gain > 0.0f) {              // 厳密に改善した = まだ登れているのでキックしない
                    stag = 0;
                    if (is_better(s.score, best.score)) best = s;   // 最良解を更新
                    continue;
                }
            }
            // else rollback(s);   // ←「先に適用して却下時に戻す」方式にする場合はここ
            // STAGNATION_LIMIT 回連続で一度も改善しなかった = 局所解に到達したとみなす
            if (++stag >= STAGNATION_LIMIT) {
                s = best;                       // 最良解に戻してから
                kick(s);                        // ランダムに大きく動かす
                stag = 0;
                g_kick++;
            }
        }
        g_iter += block;

        // ---- ブロックごとの処理 (時間計測はここだけ) ----
        const float now = timer.ms();
        if (now >= deadline_ms) break;
        tune_block(block, now - prev); prev = now;
    }
    if (is_better(best.score, s.score)) s = best;
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

    static State state; // State が大きくても良いように static 領域へ
    init_state(state);  // ★ 初期解構築
    hill_climbing_kick(state, TIME_LIMIT_MS);

    output(state);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)state.score);
    fprintf(stderr, "iter = %lld, accept = %lld (%.2f%%), time = %.1f ms\n",
            g_iter, g_accept, 100.0 * (double)g_accept / (double)max(1LL, g_iter), (double)timer.ms());
    fprintf(stderr, "kick = %lld\n", g_kick);
    // ★差分計算のバグ検出: 下の 2 つがずれていたら calc_score / apply_move が間違っている
    fprintf(stderr, "[check] diff-sum = %.3f, full = %.3f\n",
            (double)state.score, (double)full_score(state));
    return 0;
}
