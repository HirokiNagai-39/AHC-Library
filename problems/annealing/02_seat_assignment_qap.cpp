// =============================================================================
//  [焼きなまし系 02] 座席配置 (二次割当問題)
// =============================================================================
//  【問題】
//    R×C のグリッド状の座席があり、席は全部で N = R*C 個。N 人をちょうど 1 席ずつに座らせる。
//    人 i と人 j の相性は aff[i][j] (>=0、対称) で、2 人の席のマンハッタン距離を d とすると
//    aff[i][j] / (1 + d) だけ満足度が得られる。全ペアの満足度の合計を最大化せよ。
//  【入力】
//    R C
//    aff[i][0] ... aff[i][N-1]   (N 行、N = R*C)
//  【出力】
//    N 行。人 i が座る席番号 (行優先で 0..N-1)。
//  【スコア】 満足度の合計 (大きいほど良い、小数第 0 位まで)。同じ席に 2 人いれば 0 点。
//  【入力生成方法】
//    R=C=10 (N=100) 固定。各ペア (i<j) について 20% の確率で aff[i][j] = 1..10 の一様整数、
//    残り 80% は 0 とする (疎な相性表)。aff[j][i] = aff[i][j]。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     二次割当問題 (QAP) の標準形。解は「人 -> 席」の順列で、近傍は 2 人の席の入れ替え。
//     順列を保つ近傍なので実行可能性は常に満たされる。
//     素直に書くと差分計算が O(N) だが、相性表が疎 (8 割が 0) なので、
//     そこを突くと反復回数が数倍になる。
//
//   ● 状態 (State)
//     seat[人] = 席、who[席] = 人 の 2 方向の配列。
//     入れ替えのときに両方を更新するだけで済む。
//
//   ● 手の作り方
//     席 p と席 q を選び、そこに座っている 2 人を入れ替える。p の選び方で 3 種類を混ぜる。
//       (a) 近い席と入れ替える (局所的な微調整。P_NB で選ぶ)
//       (b) 相性の良い相手の隣へ引き寄せる (P_PULL で選ぶ。狙い撃ちの近傍)
//       (c) ランダムな 2 席
//     p == q になったら引き直す (無駄打ちの排除)。
//
//   ● 評価値
//     Σ_{i<j} aff[i][j] / (1 + 席間のマンハッタン距離)。最大化。
//     1/(1+距離) は前計算した行列 WEI[p][q] を引くだけにしてある。
//
//   ● 差分計算 / 高速化
//     席 p,q の 2 人を入れ替えると、変わるのは「その 2 人と他の全員の関係」だけ。
//     素直に書くと O(N) だが、相性が正の相手だけを CSR (隣接リスト形式) に畳んでおくと
//     O(相性が正の人数) = 平均で N の 1/5 程度になる。これで反復が 7.4M から 41M へ。
//     2 人の間の項は入れ替えても距離が変わらないので打ち消す (計算しなくてよい)。
//
//   ● つまずきポイント
//     ・入れ替える 2 人どうしの項を二重に数えないこと。
//     ・CSR に畳むと「相性 0 の相手」を飛ばせるが、飛ばした相手の寄与が本当に 0 か
//       (WEI が掛かっても 0 か) を確認すること。ここも [verify] が守ってくれる。
//
//   ● さらに伸ばすなら
//     ・温度・スタート回数・近傍比率をひと通り振ったがどれもノイズ範囲 (±0.3%) だった
//     ・3 人の巡回交換 (3-opt 的な近傍) を足すか、
//       「相性グラフのコミュニティごとに席の区画を割り当てる」初期解を作る方向
//
//  【改善】
//    相性表を「正の相手だけの CSR」に畳んで差分計算を O(N) から O(deg) にした (反復回数が 5.5 倍)。
//    ついでに p==q の無駄打ちを引き直しで排除し、「相性の良い相手の隣へ移る」近傍を追加した。
//    seed 0..4 合計: 変更前 6714 -> 変更後 6775 (+0.9%)  ※ seed 0..9 の 10 個すべてで改善
//
//  【採用したライブラリ】 simulated annealing/4_multi_start_annealing
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=3835 / 2_hill_climbing_kick=4029 / 3_simulated_annealing=4096 / 4_multi_start_annealing=4102 / 5_iterated_annealing=4096
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
float TEMP_START = 3.0f;   // p1: 開始温度
float TEMP_END   = 0.05f;   // p2: 終了温度
int   NUM_STARTS = 4;    // p3: スタート回数 (時間をこの数で等分する)
float P_NB       = 0.5f;   // p4: 近い席どうしを入れ替える確率
int   NB_R       = 2;      // p5: 近傍の半径 (行・列を ±NB_R ずらす)
float P_PULL     = 0.3f;   // p6: 相性の良い相手の隣へ引き寄せる近傍を選ぶ確率

void load_params() {
    pick_env("p1", TEMP_START);
    pick_env("p2", TEMP_END);
    pick_env("p3", NUM_STARTS);
    pick_env("p4", P_NB);
    pick_env("p5", NB_R);
    pick_env("p6", P_PULL);
}
#define NB_W ((uint32_t)(2 * NB_R + 1))

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化
// constexpr bool MAXIMIZE = false;    // ← スコア最小化

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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (座席配置 / 二次割当)
// #############################################################################
constexpr int MAXN = 256;

int N, R, C;
static int   AFF[MAXN][MAXN];
static float WEI[MAXN][MAXN];      // WEI[p][q] = 1/(1+席 p と q のマンハッタン距離)
// 相性表は 8 割が 0 なので、正の相手だけを CSR 形式に畳んでおく (差分計算が 5 倍速くなる)
static int16_t CTO[MAXN * MAXN];   // 相性が正の相手
static float   CWT[MAXN * MAXN];   // その相性値
static int     CBG[MAXN + 1];      // 人 i の区間は [CBG[i], CBG[i+1])

struct State {
    float   score;                 // ★必須
    int16_t seat[MAXN];            // 人 -> 席
    int16_t who[MAXN];             // 席 -> 人
};

struct Move { int p, q; };         // 席 p と席 q の人を入れ替える
static Move g_mv;

float full_score(const State &s) {
    float t = 0.0f;
    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++)
            if (AFF[i][j]) t += (float)AFF[i][j] * WEI[s.seat[i]][s.seat[j]];
    return t;
}

void init_state(State &s) {
    for (int i = 0; i < N; i++) s.seat[i] = (int16_t)i;
    for (int i = N - 1; i > 0; i--) swap(s.seat[i], s.seat[rng.next(i + 1)]);
    for (int i = 0; i < N; i++) s.who[s.seat[i]] = (int16_t)i;
    s.score = full_score(s);
}

inline void modify(State &s) {
    g_mv.p = (int)rng.next((uint32_t)N);
    const float r = rng.nextf();
    do {
        if (r < P_NB) {                             // (a) 近い席と入れ替える (微調整)
            const int pr = g_mv.p / C, pc = g_mv.p % C;
            const int nr = min(R - 1, max(0, pr + (int)rng.next(NB_W) - NB_R));
            const int nc = min(C - 1, max(0, pc + (int)rng.next(NB_W) - NB_R));
            g_mv.q = nr * C + nc;
        } else if (r < P_NB + P_PULL) {             // (b) 相性の良い相手の隣へ引き寄せる
            const int i = s.who[g_mv.p];
            const int b = CBG[i], e = CBG[i + 1];
            if (e > b) {
                const int sk = s.seat[CTO[b + (int)rng.next((uint32_t)(e - b))]];
                const int nr = min(R - 1, max(0, sk / C + (int)rng.next(3) - 1));
                const int nc = min(C - 1, max(0, sk % C + (int)rng.next(3) - 1));
                g_mv.q = nr * C + nc;
            } else g_mv.q = (int)rng.next((uint32_t)N);
        } else {                                    // (c) 完全にランダムな 2 席
            g_mv.q = (int)rng.next((uint32_t)N);
        }
    } while (g_mv.q == g_mv.p);                     // p == q は完全な無駄打ちなので引き直す
}

inline float calc_score(State &s) {
    const int p = g_mv.p, q = g_mv.q;
    const int i = s.who[p], j = s.who[q];
    const float *wp = WEI[p], *wq = WEI[q];
    float d = 0.0f;
    for (int e = CBG[i], ee = CBG[i + 1]; e < ee; e++) {   // 相性が正の相手だけ見る
        const int k = CTO[e];
        if (k == j) continue;                              // ペア (i,j) は入れ替えても距離が変わらない
        const int sk = s.seat[k];
        d += CWT[e] * (wq[sk] - wp[sk]);
    }
    for (int e = CBG[j], ee = CBG[j + 1]; e < ee; e++) {
        const int k = CTO[e];
        if (k == i) continue;
        const int sk = s.seat[k];
        d += CWT[e] * (wp[sk] - wq[sk]);
    }
    return d;
}

inline void apply_move(State &s) {
    const int p = g_mv.p, q = g_mv.q;
    const int i = s.who[p], j = s.who[q];
    s.seat[i] = (int16_t)q; s.seat[j] = (int16_t)p;
    s.who[p] = (int16_t)j;  s.who[q] = (int16_t)i;
}

void read_input() {
    if (scanf("%d %d", &R, &C) == 2 && R >= 1) {
        N = R * C; N = min(N, MAXN);
        for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) scanf("%d", &AFF[i][j]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 202);
        R = 10; C = 10; N = R * C;
        for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) AFF[i][j] = 0;
        for (int i = 0; i < N; i++)
            for (int j = i + 1; j < N; j++)
                if (g.nextf() < 0.2f) { const int w = 1 + (int)g.next(10); AFF[i][j] = w; AFF[j][i] = w; }
    }
    for (int p = 0; p < N; p++)
        for (int q = 0; q < N; q++) {
            const int d = abs(p / C - q / C) + abs(p % C - q % C);
            WEI[p][q] = 1.0f / (1.0f + (float)d);
        }
    // ---- 相性が正の相手だけを CSR に詰める ----
    int c = 0;
    for (int i = 0; i < N; i++) {
        CBG[i] = c;
        for (int k = 0; k < N; k++) if (k != i && AFF[i][k]) { CTO[c] = (int16_t)k; CWT[c] = (float)AFF[i][k]; c++; }
    }
    CBG[N] = c;
}

void output(const State &s) {
    string r; r.reserve((size_t)N * 4);
    for (int i = 0; i < N; i++) { r += to_string((int)s.seat[i]); r += '\n'; }
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
