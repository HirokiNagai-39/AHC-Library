// =============================================================================
//  [焼きなまし系 10] 容量制約付き配送計画 (CVRP)
// =============================================================================
//  【問題】
//    地点 0 が車庫、地点 1..N-1 が顧客で、顧客 i の需要は q_i。
//    容量 Q の車が K 台あり、それぞれ車庫を出発して顧客を回り車庫に戻る。
//    すべての顧客をちょうど 1 台が 1 回だけ訪問し、1 台の需要合計は Q 以下でなければならない。
//    コスト = 総移動距離 + 1000 * (容量超過量の合計) を最小化せよ。
//  【入力】
//    N K Q
//    x_i y_i q_i   (N 行。i=0 は車庫で q_0=0)
//  【出力】
//    K 行。各行は "m p_1 ... p_m" (その車が訪問する顧客を訪問順に)。
//  【スコア】 上記コスト (小さいほど良い)。全顧客がちょうど 1 回現れなければ 0 点。
//  【入力生成方法】
//    N=201 (車庫 1 + 顧客 200), K=8, Q=250 固定。x,y は [0,1000] の一様実数、q_i は 5..30 の一様整数。
//    需要合計の期待値は約 3500 で総容量 2000 を超えるので、容量ペナルティを避けきれない
//    …とはならないよう Q=250*8=2000 に対し q は 5..30 (平均 17.5、合計約 3500)。
//    ※ 生成時に「合計需要が総容量の 0.9 倍」になるよう q を一律スケールするので必ず実行可能。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     容量制約付き配送計画 (CVRP) は焼きなましの定番。ポイントは 2 つ。
//       ・容量超過を禁止せずペナルティにする。禁止すると近傍がほとんど動けない
//       ・でたらめな移動はまず却下されるので、移動先を「近い顧客のとなり」に絞る
//         (granular neighborhood)。これで採択率が 6% から 20% に上がる
//     「速くする」より「当たる手を提案する」方が効く問題で、実際 10 倍の時間を与えても
//     スコアが変わらない (= 完全に収束している) ことを確認したうえで近傍を作り直した。
//
//   ● 状態 (State)
//     全車の訪問順を 1 本の配列 seq[] に連結し、車ごとの長さ len[] と積載量 load[] を別に持つ。
//     車ごとに配列を分けるより State が小さく、コピーが速い。
//
//   ● 手の作り方
//     4 種類を確率で使い分ける。
//       ・or-opt: 連続 L 人 (L <= SEG_LEN) をまとめて別の位置へ移す (反転して入れることもある)
//       ・swap  : 2 人を入れ替える
//       ・2-opt : 1 台のルート内で区間を反転する
//       ・2-opt*: 2 台のルートの後半どうしを交換する (積載バランスを大きく変えられる)
//     移動先・交換相手は「その顧客に近い顧客のとなり」から選ぶ (近傍リストは前計算)。
//
//   ● 評価値
//     総移動距離 + 1000 × (容量超過量の合計)。最小化。
//     超過ペナルティが十分大きいので、温度が下がると自然に実行可能解へ落ちる。
//
//   ● 差分計算 / 高速化
//     どの近傍も「つなぎ変わる辺」と「積載量」しか変わらないので、差分は O(1)。
//     全ルートを舐め直す O(N+K) の全計算に対して 5 倍以上速く、反復数が 11M から 25M になった。
//     初期解はスイープ法 (デポから見た方位角順に詰める) にしてある。
//
//   ● つまずきポイント
//     ・or-opt で「同じルート内での移動」と「別ルートへの移動」で場合分けを間違えやすい。
//       特に移動元と移動先が隣接している場合。ここは [verify] で全計算と突き合わせて確認できる。
//     ・2-opt* は積載量が大きく動くので、ペナルティの差分を忘れると壊れる。
//
//   ● さらに伸ばすなら
//     ・多点スタートや繰り返し焼きなましは逆に悪化した (1 回の焼きなましで完全収束しているため)
//     ・伸ばすなら「Or-opt の区間長をもっと伸ばす」「破壊と再構築 (ruin & recreate) を入れる」方向
//
//  【改善】
//    (1) 全再計算 O(N+K) をルート端の辺だけ見る O(1) 差分に変更 (反復数 11M -> 25M)。
//    (2) 近傍にルート内 2-opt と 2-opt* を追加。初期解をスイープ法 (方位角順) に変更。
//    (3) 移動先を「近い顧客のとなり」から選ぶ granular neighborhood を導入。
//    (4) 温度を再調整 (300/1 -> 40/2)。
//    seed 0..4 合計: 変更前 78304 -> 変更後 75724 (-3.3%)
//
//  【採用したライブラリ】 simulated annealing/3_simulated_annealing
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=59240 / 2_hill_climbing_kick=52488 / 3_simulated_annealing=47595 / 4_multi_start_annealing=48567 / 5_iterated_annealing=48913
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
float TEMP_START = 40.0f;    // p1: 開始温度
float TEMP_END   = 2.0f;     // p2: 終了温度
float P_REL      = 0.55f;    // p3: 区間移動 (or-opt) を選ぶ確率
float P_SWAP     = 0.15f;    // p4: 2 顧客の入れ替えを選ぶ確率
float P_2OPT     = 0.15f;    // p5: ルート内 2-opt を選ぶ確率 (残りは 2-opt*)
int   SEG_LEN    = 3;        // p6: 区間移動で動かす連続顧客数の上限
int   INIT_MODE  = 1;        // p7: 初期解 0=ランダム分割 1=スイープ法
float P_GRAN     = 0.8f;     // p8: 行き先を「近い顧客のとなり」から選ぶ確率
int   NUM_NB     = 10;       // p9: 近い顧客リストの長さ

void load_params() {
    pick_env("p1", TEMP_START);
    pick_env("p2", TEMP_END);
    pick_env("p3", P_REL);
    pick_env("p4", P_SWAP);
    pick_env("p5", P_2OPT);
    pick_env("p6", SEG_LEN);
    pick_env("p7", INIT_MODE);
    pick_env("p8", P_GRAN);
    pick_env("p9", NUM_NB);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (容量制約付き配送計画)
// #############################################################################
constexpr int MAXN = 400, MAXK = 16;
constexpr float CAP_PEN = 1000.0f;

int N, K, T;                       // T = N-1 (顧客数)
float QCAP;
static int   QD[MAXN];
static float D[MAXN * MAXN];
static float ANG[MAXN];               // 車庫から見た方位角 (スイープ法の初期解用)
constexpr int MAXNL = 24;
static int16_t NB[MAXN][MAXNL];       // 各顧客に近い顧客のリスト (近傍の行き先をここから選ぶ)
static int16_t POSOF[MAXN];           // 顧客 -> seq 上の位置
static inline float dist(int i, int j) { return D[i * N + j]; }
static inline float pen(float load) { return load > QCAP ? CAP_PEN * (load - QCAP) : 0.0f; }

struct State {
    float   score;
    int16_t seq[MAXN];            // 全車のルートを連結したもの (顧客のみ)
    int16_t len[MAXK];
    float   load[MAXK];           // 各車の積載量 (len/seq から決まる派生情報)
};

static inline float eval_seq(const int16_t *seq, const int16_t *len) {
    float t = 0.0f;
    int base = 0;
    for (int k = 0; k < K; k++) {
        int prev = 0; float load = 0.0f;
        for (int q = 0; q < len[k]; q++) {
            const int c = seq[base + q];
            t += dist(prev, c); load += (float)QD[c]; prev = c;
        }
        t += dist(prev, 0);
        if (load > QCAP) t += CAP_PEN * (load - QCAP);
        base += len[k];
    }
    return t;
}

float full_score(const State &s) { return eval_seq(s.seq, s.len); }

static inline void recalc_load(State &s) {
    int base = 0;
    for (int k = 0; k < K; k++) {
        float l = 0.0f;
        for (int q = 0; q < s.len[k]; q++) l += (float)QD[s.seq[base + q]];
        s.load[k] = l; base += s.len[k];
    }
}
static const State *g_pos_src = nullptr;
static inline void posof_range(const State &s, int a, int b) {
    if (a < 0) a = 0;
    if (b > T) b = T;
    for (int g = a; g < b; g++) POSOF[s.seq[g]] = (int16_t)g;
}
static inline void ensure_posof(const State &s) {
    if (g_pos_src != &s) { posof_range(s, 0, T); g_pos_src = &s; }
}


void init_state(State &s) {
    static int16_t p[MAXN];
    for (int i = 0; i < T; i++) p[i] = (int16_t)(i + 1);
    if (INIT_MODE == 1) {
        // スイープ法: 車庫から見た方位角の順に並べる。
        // 各車のルートが扇形にまとまるので、ランダム分割よりずっと短い初期解になる。
        sort(p, p + T, [](int16_t a2, int16_t b2) { return ANG[a2] < ANG[b2]; });
    } else {
        for (int i = T - 1; i > 0; i--) swap(p[i], p[rng.next(i + 1)]);
    }
    // 需要合計がほぼ均等になるように K 分割する
    float tot = 0.0f;
    for (int i = 0; i < T; i++) tot += (float)QD[p[i]];
    const float per = tot / (float)K;
    int idx = 0;
    for (int k = 0; k < K; k++) {
        int c = 0; float l = 0.0f;
        const int rest = K - k - 1;
        while (idx < T && (k == K - 1 || (l < per && T - idx > rest))) {
            l += (float)QD[p[idx]];
            s.seq[idx] = p[idx]; idx++; c++;
        }
        s.len[k] = (int16_t)c;
    }
    recalc_load(s);
    posof_range(s, 0, T); g_pos_src = &s;
    s.score = eval_seq(s.seq, s.len);
}

// ---- 近傍 ----
//  0: 連続 L 人 (L<=SEG_LEN) をまとめて別の位置 (別の車でも良い) へ移す (向きの反転もあり)
//  1: 2 人を入れ替える
//  2: 1 台のルート内で区間を反転する (2-opt)
//  3: 2 台のルートの後半どうしを丸ごと交換する (2-opt*)
// いずれも「つなぎ変わる辺」と「積載量」だけ見れば良いので差分はほぼ O(1)。
struct Move { int type, k1, p1, L, k2, q, rev, a, b, k2b, p2, i2, j2; };
static Move g_mv;
static float g_diff = 0.0f;
static int   g_since = 0;

static inline int route_of(const State &s, int g, int &base) {
    int k = 0; base = 0;
    while (g >= base + s.len[k]) { base += s.len[k]; k++; }
    return k;
}
static inline int base_of(const State &s, int k) {
    int b = 0; for (int i = 0; i < k; i++) b += s.len[i]; return b;
}
inline void modify(State &s) {
    ensure_posof(s);
    const float r = rng.nextf();
    if (r < P_REL) {                                        // 区間移動
        g_mv.type = 0;
        int base1; const int a = (int)rng.next((uint32_t)T);
        const int k1 = route_of(s, a, base1);
        const int p1 = a - base1;
        int L = 1 + (int)rng.next((uint32_t)SEG_LEN);
        if (p1 + L > s.len[k1]) L = s.len[k1] - p1;
        g_mv.k1 = k1; g_mv.p1 = p1; g_mv.L = L;
        g_mv.rev = (L >= 2) ? (int)rng.next(2) : 0;
        int k2, q;
        const int gj = (rng.nextf() < P_GRAN)
                     ? (int)POSOF[NB[s.seq[a]][rng.next((uint32_t)NUM_NB)]] : -1;
        if (gj >= 0 && (gj < a || gj >= a + L)) {           // 近い顧客のとなりへ入れる
            int base2; k2 = route_of(s, gj, base2);
            int pj = gj - base2;
            if (k2 == k1 && pj > p1) pj -= L;               // 抜いた後の並びでの位置
            q = pj + (int)rng.next(2);
        } else {                                            // ランダムな位置へ
            k2 = (int)rng.next((uint32_t)K);
            const int room = (int)s.len[k2] - (k2 == k1 ? L : 0);
            q = (int)rng.next((uint32_t)(room + 1));
        }
        const int room = (int)s.len[k2] - (k2 == k1 ? L : 0);
        if (q < 0) q = 0; else if (q > room) q = room;
        g_mv.k2 = k2; g_mv.q = q;
    } else if (r < P_REL + P_SWAP) {                        // 2 顧客の入れ替え
        g_mv.type = 1;
        int a = (int)rng.next((uint32_t)T), b;
        if (rng.nextf() < P_GRAN) b = (int)POSOF[NB[s.seq[a]][rng.next((uint32_t)NUM_NB)]];
        else                      b = (int)rng.next((uint32_t)T);
        if (a > b) swap(a, b);
        g_mv.a = a; g_mv.b = b;
    } else if (r < P_REL + P_SWAP + P_2OPT) {               // ルート内 2-opt
        g_mv.type = 2;
        int base; const int a = (int)rng.next((uint32_t)T);
        const int k = route_of(s, a, base);
        int p1 = a - base, p2;
        if (rng.nextf() < P_GRAN) {                         // 近い顧客と隣り合うように反転する
            const int gj = (int)POSOF[NB[s.seq[a]][rng.next((uint32_t)NUM_NB)]];
            int b2; if (route_of(s, gj, b2) != k) { g_mv.p1 = g_mv.p2 = p1; g_mv.k1 = k; return; }
            p2 = gj - base;
            if (p1 > p2) swap(p1, p2);
            p1++;                                           // [p1+1, p2] を反転すると辺 (a, j) ができる
        } else {
            p2 = (int)rng.next((uint32_t)s.len[k]);
            if (p1 > p2) swap(p1, p2);
        }
        g_mv.k1 = k; g_mv.p1 = p1; g_mv.p2 = p2;
    } else {                                                // 2-opt*: 2 台の後半を交換
        g_mv.type = 3;
        if (rng.nextf() < P_GRAN) {
            int b1, b2;
            const int a = (int)rng.next((uint32_t)T);
            const int k1 = route_of(s, a, b1);
            const int gj = (int)POSOF[NB[s.seq[a]][rng.next((uint32_t)NUM_NB)]];
            const int k2 = route_of(s, gj, b2);
            if (k1 == k2) { g_mv.k1 = k1; g_mv.k2 = k1 == 0 ? 1 % K : 0; g_mv.i2 = s.len[g_mv.k1] - 1; g_mv.j2 = s.len[g_mv.k2] - 1; return; }
            g_mv.k1 = k1; g_mv.k2 = k2;
            g_mv.i2 = a - b1;                               // a の直後で切る
            g_mv.j2 = gj - b2 - 1;                          // j の直前で切る → 辺 (a, j) ができる
        } else {
            const int k1 = (int)rng.next((uint32_t)K);
            const int k2 = (k1 + 1 + (int)rng.next((uint32_t)(K - 1))) % K;
            g_mv.k1 = k1; g_mv.k2 = k2;
            g_mv.i2 = (int)rng.next((uint32_t)(s.len[k1] + 1)) - 1;
            g_mv.j2 = (int)rng.next((uint32_t)(s.len[k2] + 1)) - 1;
        }
    }
}

inline float calc_score(State &s) {
    if (g_mv.type == 3) {                                   // 2-opt* (2 台の後半を交換)
        const int k1 = g_mv.k1, k2 = g_mv.k2, i = g_mv.i2, j = g_mv.j2;
        const int b1 = base_of(s, k1), l1 = s.len[k1];
        const int b2 = base_of(s, k2), l2 = s.len[k2];
        if (i == l1 - 1 && j == l2 - 1) return (g_diff = 0.0f);
        const int a1 = i < 0 ? 0 : (int)s.seq[b1 + i], e1 = i + 1 < l1 ? (int)s.seq[b1 + i + 1] : 0;
        const int a2 = j < 0 ? 0 : (int)s.seq[b2 + j], e2 = j + 1 < l2 ? (int)s.seq[b2 + j + 1] : 0;
        float d = dist(a1, e2) + dist(a2, e1) - dist(a1, e1) - dist(a2, e2);
        float h1 = 0.0f, h2 = 0.0f;                          // 前半の需要
        for (int q = 0; q <= i; q++) h1 += (float)QD[s.seq[b1 + q]];
        for (int q = 0; q <= j; q++) h2 += (float)QD[s.seq[b2 + q]];
        const float t1 = s.load[k1] - h1, t2 = s.load[k2] - h2;
        d += pen(h1 + t2) + pen(h2 + t1) - pen(s.load[k1]) - pen(s.load[k2]);
        return (g_diff = d);
    }
    if (g_mv.type == 2) {                                   // ルート内 2-opt
        const int k = g_mv.k1, p1 = g_mv.p1, p2 = g_mv.p2;
        if (p1 == p2) return (g_diff = 0.0f);
        const int base = base_of(s, k), ln = s.len[k];
        const int A = p1 == 0 ? 0 : (int)s.seq[base + p1 - 1];
        const int B = p2 == ln - 1 ? 0 : (int)s.seq[base + p2 + 1];
        const int c1 = (int)s.seq[base + p1], c2 = (int)s.seq[base + p2];
        return (g_diff = dist(A, c2) + dist(c1, B) - dist(A, c1) - dist(c2, B));
    }
    if (g_mv.type == 1) {                                   // 2 顧客の入れ替え
        const int a = g_mv.a, b = g_mv.b;
        if (a == b) return (g_diff = 0.0f);
        int base1, base2;
        const int k1 = route_of(s, a, base1), k2 = route_of(s, b, base2);
        const int p1 = a - base1, p2 = b - base2;
        const int l1 = s.len[k1], l2 = s.len[k2];
        const int c1 = (int)s.seq[a], c2 = (int)s.seq[b];
        const int A1 = p1 == 0 ? 0 : (int)s.seq[a - 1], B1 = p1 == l1 - 1 ? 0 : (int)s.seq[a + 1];
        const int A2 = p2 == 0 ? 0 : (int)s.seq[b - 1], B2 = p2 == l2 - 1 ? 0 : (int)s.seq[b + 1];
        float d;
        if (k1 == k2 && p2 == p1 + 1)                       // 隣り合っている場合
            d = dist(A1, c2) + dist(c1, B2) - dist(A1, c1) - dist(c2, B2);
        else
            d = dist(A1, c2) + dist(c2, B1) - dist(A1, c1) - dist(c1, B1)
              + dist(A2, c1) + dist(c1, B2) - dist(A2, c2) - dist(c2, B2);
        if (k1 != k2) {
            const float q1 = (float)QD[c1], q2 = (float)QD[c2];
            d += pen(s.load[k1] - q1 + q2) - pen(s.load[k1])
               + pen(s.load[k2] - q2 + q1) - pen(s.load[k2]);
        }
        g_mv.k1 = k1; g_mv.k2b = k2;
        return (g_diff = d);
    }
    // 区間移動
    const int k1 = g_mv.k1, p1 = g_mv.p1, L = g_mv.L, k2 = g_mv.k2, q = g_mv.q, rev = g_mv.rev;
    const int base1 = base_of(s, k1), l1 = s.len[k1];
    const int cf = (int)s.seq[base1 + p1], cl = (int)s.seq[base1 + p1 + L - 1];
    const int A = p1 == 0 ? 0 : (int)s.seq[base1 + p1 - 1];
    const int B = p1 + L == l1 ? 0 : (int)s.seq[base1 + p1 + L];
    float d = dist(A, B) - dist(A, cf) - dist(cl, B);        // 抜き取る側
    int X, Y;
    if (k2 == k1) {                                          // 同じルート内 (抜いた後の並びで見る)
        const int lr = l1 - L;
        if (q == p1 && !rev) return (g_diff = 0.0f);
        X = q == 0  ? 0 : (int)s.seq[base1 + (q - 1 < p1 ? q - 1 : q - 1 + L)];
        Y = q == lr ? 0 : (int)s.seq[base1 + (q     < p1 ? q     : q     + L)];
    } else {
        const int base2 = base_of(s, k2), l2 = s.len[k2];
        X = q == 0  ? 0 : (int)s.seq[base2 + q - 1];
        Y = q == l2 ? 0 : (int)s.seq[base2 + q];
        float qs = 0.0f;
        for (int u = 0; u < L; u++) qs += (float)QD[s.seq[base1 + p1 + u]];
        d += pen(s.load[k1] - qs) - pen(s.load[k1]) + pen(s.load[k2] + qs) - pen(s.load[k2]);
        g_mv.a = (int)qs;                                    // 積載量の移動分 (整数なので誤差なし)
    }
    if (rev) d += dist(X, cl) + dist(cf, Y) - dist(X, Y);
    else     d += dist(X, cf) + dist(cl, Y) - dist(X, Y);
    return (g_diff = d);
}

inline void apply_move(State &s) {
    if (g_mv.type == 3) {
        const int k1 = g_mv.k1, k2 = g_mv.k2, i = g_mv.i2, j = g_mv.j2;
        const int b1 = base_of(s, k1), l1 = s.len[k1];
        const int b2 = base_of(s, k2), l2 = s.len[k2];
        static int16_t nt[MAXN]; static int16_t nl[MAXK];
        int u = 0;
        for (int k = 0; k < K; k++) {
            const int bk = base_of(s, k);
            if (k == k1) {
                for (int q = 0; q <= i; q++)      nt[u++] = s.seq[b1 + q];
                for (int q = j + 1; q < l2; q++)  nt[u++] = s.seq[b2 + q];
                nl[k] = (int16_t)(i + 1 + l2 - j - 1);
            } else if (k == k2) {
                for (int q = 0; q <= j; q++)      nt[u++] = s.seq[b2 + q];
                for (int q = i + 1; q < l1; q++)  nt[u++] = s.seq[b1 + q];
                nl[k] = (int16_t)(j + 1 + l1 - i - 1);
            } else {
                for (int q = 0; q < s.len[k]; q++) nt[u++] = s.seq[bk + q];
                nl[k] = s.len[k];
            }
        }
        memcpy(s.seq, nt, sizeof(int16_t) * (size_t)T);
        memcpy(s.len, nl, sizeof(int16_t) * (size_t)K);
        recalc_load(s);                                      // どうせ O(N) なので積載量も作り直す
        posof_range(s, 0, T);
    } else if (g_mv.type == 2) {
        const int base = base_of(s, g_mv.k1);
        reverse(s.seq + base + g_mv.p1, s.seq + base + g_mv.p2 + 1);
        posof_range(s, base + g_mv.p1, base + g_mv.p2 + 1);
    } else if (g_mv.type == 1) {
        const int k1 = g_mv.k1, k2 = g_mv.k2b;
        const int c1 = (int)s.seq[g_mv.a], c2 = (int)s.seq[g_mv.b];
        swap(s.seq[g_mv.a], s.seq[g_mv.b]);
        POSOF[c1] = (int16_t)g_mv.b; POSOF[c2] = (int16_t)g_mv.a;
        if (k1 != k2) {
            const float q1 = (float)QD[c1], q2 = (float)QD[c2];
            s.load[k1] += q2 - q1; s.load[k2] += q1 - q2;
        }
    } else {
        const int k1 = g_mv.k1, p1 = g_mv.p1, L = g_mv.L, k2 = g_mv.k2, q = g_mv.q;
        const int base1 = base_of(s, k1);
        static int16_t buf[8];
        memcpy(buf, s.seq + base1 + p1, sizeof(int16_t) * (size_t)L);
        if (g_mv.rev) reverse(buf, buf + L);
        const int g1 = base1 + p1;
        memmove(s.seq + g1, s.seq + g1 + L, sizeof(int16_t) * (size_t)(T - g1 - L));
        s.len[k1] -= (int16_t)L;
        int base2 = 0;
        for (int i = 0; i < k2; i++) base2 += s.len[i];      // 抜いた後の並びでの開始位置
        const int gi = base2 + q;
        memmove(s.seq + gi + L, s.seq + gi, sizeof(int16_t) * (size_t)(T - L - gi));
        memcpy(s.seq + gi, buf, sizeof(int16_t) * (size_t)L);
        s.len[k2] += (int16_t)L;
        posof_range(s, min(g1, gi), max(g1, gi) + L);
        if (k1 != k2) { const float qs = (float)g_mv.a; s.load[k1] -= qs; s.load[k2] += qs; }
    }
    // float の誤差が溜まらないよう、たまに厳密値へ戻す (呼び出し側が直後に += g_diff する)
    if (++g_since >= 512) { g_since = 0; s.score = eval_seq(s.seq, s.len) - g_diff; }
}

void read_input() {
    static float px[MAXN], py[MAXN];
    if (scanf("%d %d %f", &N, &K, &QCAP) == 3 && N >= 2) {
        N = min(N, MAXN); K = min(K, MAXK);
        for (int i = 0; i < N; i++) scanf("%f %f %d", &px[i], &py[i], &QD[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 210);
        N = 201; K = 8; QCAP = 250.0f;
        long long tot = 0;
        for (int i = 0; i < N; i++) { px[i] = g.nextf() * 1000.0f; py[i] = g.nextf() * 1000.0f; QD[i] = 5 + (int)g.next(26); tot += QD[i]; }
        QD[0] = 0;
        const double scale = 0.9 * (double)QCAP * K / (double)tot;
        for (int i = 1; i < N; i++) QD[i] = max(1, (int)lround((double)QD[i] * scale));
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            const float dx = px[i] - px[j], dy = py[i] - py[j];
            D[i * N + j] = sqrtf(dx * dx + dy * dy);
        }
    T = N - 1;
    for (int i = 0; i < N; i++) ANG[i] = atan2f(py[i] - py[0], px[i] - px[0]);
    if (NUM_NB > MAXNL) NUM_NB = MAXNL;
    if (NUM_NB > T - 1) NUM_NB = max(1, T - 1);
    {   // 各顧客について近い顧客を NUM_NB 個そろえる (近傍の行き先をここから選ぶと当たりが増える)
        static int idx[MAXN];
        for (int i = 1; i < N; i++) {
            int m = 0;
            for (int j = 1; j < N; j++) if (j != i) idx[m++] = j;
            const float *Dp = D + i * N;
            partial_sort(idx, idx + NUM_NB, idx + m, [Dp](int a2, int b2) { return Dp[a2] < Dp[b2]; });
            for (int q = 0; q < NUM_NB; q++) NB[i][q] = (int16_t)idx[q];
        }
    }
}

void output(const State &s) {
    int base = 0;
    for (int k = 0; k < K; k++) {
        printf("%d", (int)s.len[k]);
        for (int q = 0; q < s.len[k]; q++) printf(" %d", (int)s.seq[base + q]);
        printf("\n");
        base += s.len[k];
    }
}
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
