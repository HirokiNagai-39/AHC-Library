// =============================================================================
//  [貪欲系 04] アイテム合成 (crafting chain)
// =============================================================================
//  【問題】
//    M 種類のアイテムがあり、アイテム i の価値は val_i、初期在庫は st_i 個。
//    R 個のレシピがあり、レシピ r は「アイテム a_r を x_r 個 と アイテム b_r を y_r 個 消費して
//    アイテム c_r を 1 個 作る」。同じレシピは何度でも使える。
//    合成操作を最大 T 回まで行い、最後に手元に残ったアイテムの価値の合計を最大化せよ。
//    ※ 目先は損でも、その中間素材を使うともっと高いアイテムが作れることがある。
//  【入力】
//    M R T
//    val_0 ... val_{M-1}
//    st_0 ... st_{M-1}
//    a x b y c    (R 行)
//  【出力】
//    使ったレシピ番号を 1 行に 1 つ、実行順に出力する (行数が操作回数)。
//  【スコア】 最終在庫の価値合計 (大きいほど良い)。在庫不足のレシピを使ったら 0 点。
//  【入力生成方法】
//    M=40 (レベル 0..3 に 10 種類ずつ)、R=80、T=400 固定。
//    val_i は レベル lv の種類について 1..40*(lv+1) の一様整数。
//    初期在庫はレベル 0 のみ 60 個、他は 0 個。
//    レシピ r は レベル d (0..2) をランダムに選び、レベル d の 2 種類 a,b と
//    レベル d+1 の 1 種類 c をランダムに選び、x,y は 1..3 の一様整数。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「損な合成もできてしまう」のがこの問題の罠。ライブラリの貪欲は
//     「実行できる手があれば必ず実行する」構造なので、放っておくと損なレシピを在庫が尽きるまで打ち続け、
//     何もしないより価値が減る。まず「打たない」という選択ができるようにするのが最優先。
//     そのうえで、中間素材は単体では価値が低くても「その先の合成」で回収できるので、
//     目先の価値差ではなく「先まで見込んだ利益」で判断する必要がある。
//
//   ● 状態 (State)
//     各アイテムの在庫数と、これまでに実行したレシピ列 (出力用)。
//
//   ● 手の作り方
//     前計算で各レシピの見込み利益
//       PROF[r] = VAL[c] + BONUS_W * BONUS[c] - x * VAL[a] - y * VAL[b]
//     を求め、PROF > MARGIN のレシピだけを候補にする。
//     候補が無くなればそこで打ち止まりになるので、「作らない」が自然に選べる。
//
//     BONUS[i] は「アイテム i を 1 個持っていると、この先の合成でさらに稼げる見込み額」で、
//     レベルの高い方から低い方へ伝播させる後ろ向き DP で作る (build_potential)。
//       BONUS[i] = 0 で初期化し、d = 最大レベル-1, ..., 0 の順に、材料 a がレベル d のレシピ r について
//         p = VAL[c] + BONUS_W * BONUS[c] - x * VAL[a] - y * VAL[b]   (r 自身の見込み利益)
//         p > 0 なら  BONUS[a] = max(BONUS[a], p / x),  BONUS[b] = max(BONUS[b], p / y)
//       つまり「利益 p のレシピが材料 a を x 個食うなら、a 1 個の価値は p/x だけ上乗せできる」と考え、
//       そのアイテムの一番おいしい使い道 (max) を採る。c は必ず 1 レベル上なので BONUS[c] は計算済み。
//
//     ★ただし既定は BONUS_W = 0 で、この見込み項は切ってある。
//       BONUS_W = 1 にすると seed 0..4 で -1.2% と悪化した (max を採るので見込みを過大評価しやすく、
//       材料が尽きて実現しない連鎖にも高い値が付くため)。
//       連鎖の先読みは、この項ではなく後述のプレイアウト (HORIZON=60 手) が担当している。
//       BONUS_W は p1 として残してあるので、問題を変えたときに効くかどうかは optuna で確かめられる。
//     さらに、選んだ手の先を貪欲で HORIZON 手プレイアウトして結果を比べ、
//     いちばん良かった手を実際に採用する (ローリングホライゾン)。
//     合成は連鎖するので、1 手先だけ見る貪欲では中間素材の価値を過小評価してしまう。
//
//   ● 評価値
//     PROF[r] / (x + y)^EFF  … 「材料 1 個あたりの見込み利益」。
//     この問題でいちばん足りない資源はレベル 0 の初期在庫なので、利益の絶対額ではなく
//     材料あたりのコスパで選んだ方が、同じ在庫からより多くの利益を取り出せる。
//
//   ● 差分計算 / 高速化
//     スコアの差分は「作る物の価値 - 材料の価値」で O(1)。
//     候補はレシピ数ぶんしかないので、1 手の走査は軽い。
//     プレイアウトの長さ HORIZON がコストを決めるので、そこで質と回数のバランスを取る。
//
//   ● つまずきポイント
//     ・初版は損なレシピを打ち続けて初期在庫より価値を減らしていた (seed 0 で 12300 → 8941)。
//       「候補を絞ることで貪欲が止まれるようにする」だけでスコアがほぼ倍になった。
//     ・評価値を「作るアイテムのレベルが高いほど良い」という下駄にしていたのも良くなかった。
//       レベルは利益の代理指標としては粗すぎる。
//
//   ● さらに伸ばすなら
//     ・見込み利益をレベルごとの動的計画法で厳密に計算する
//     ・「どのレベル 0 素材が何個必要か」を線形計画で解き、その双対価格を評価値に使う
//
//  【改善】
//    ・元の実装は「実行できるレシピがあれば必ず実行する」ので、損なレシピを在庫が尽きるまで打ち続け、
//      初期在庫より価値を減らしていた。見込み利益が 0 以下のレシピを候補から外し、
//      候補が無くなったら止まる (= 作らない) ようにしたのが一番効いた
//    ・評価値を「レベルの下駄」から「材料 1 個あたりの見込み利益」に変えた
//    ・プレイアウトを長くした (HORIZON 20 -> 60)
//    seed 0..4 合計: 変更前 38081 -> 変更後 71640 (+88.1%)
//
//  【採用したライブラリ】 greedy/3_rolling_horizon
//    実測比較 (seed 0,1,2 の合計スコア): 1_greedy=18751 / 3_rolling_horizon=18874
//    改善後に再比較しても 3_rolling_horizon の方が良かった (seed 0..4 合計: 1_greedy=71252 / 3_rolling_horizon=71560)
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
float BONUS_W   = 0.0f;    // p1: 連鎖の見込み (BONUS) をどれだけ信じるか (0 で目先の利益だけ)
float MARGIN    = 0.0f;    // p2: この見込み利益以下のレシピは実行しない (打ち切りの閾値)
int   HORIZON   = 60;      // p3: プレイアウトで何手先まで進めるか (-1 なら最後まで)
float NOISE     = 0.05f;   // p4: プレイアウトのノイズ幅
float EFF       = 1.0f;    // p5: 材料 1 個あたりの利益で選ぶ度合い (0 なら利益の絶対額で選ぶ)

void load_params() {
    pick_env("p1", BONUS_W);
    pick_env("p2", MARGIN);
    pick_env("p3", HORIZON);
    pick_env("p4", NOISE);
    pick_env("p5", EFF);
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
// (この手法は探索設定より「評価関数」と「プレイアウトの長さ」が効く)

// 統計 (デバッグ用。不要なら消して良い)
static ll g_trials = 0, g_steps = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (アイテム合成)
// #############################################################################
constexpr int MAXM = 200, MAXR = 400, MAXOP = 2000;
constexpr int MAX_CAND = MAXR;             // ★候補はレシピ全部

int M, R, TMAX;
static int VALI[MAXM], LV[MAXM], ST0[MAXM];
static int RA[MAXR], RX[MAXR], RB[MAXR], RY[MAXR], RC[MAXR];
// ★前計算: BONUS[i] = 「アイテム i を 1 個持っていると、この先の合成でさらに稼げる見込み額」
//           PROF[r]  = 「レシピ r の見込み利益」= (作る物の価値 + その見込み) - 材料の価値
static float BONUS[MAXM], PROF[MAXR], EFFV[MAXR];

struct Move { int16_t rec; };

struct State {
    float   score;
    int32_t turn;
    int32_t stock[MAXM];
    int16_t ops[MAXOP];
};

void init_state(State &s) {
    s.turn = 0;
    float v = 0.0f;
    for (int i = 0; i < M; i++) { s.stock[i] = ST0[i]; v += (float)ST0[i] * (float)VALI[i]; }
    s.score = v;
}

// ★見込み利益が MARGIN 以下のレシピは最初から候補にしない。
//   元の実装は「実行できるレシピが 1 つでもあれば必ず実行する」ので、
//   損なレシピを在庫が尽きるまで打ち続けて初期在庫より価値を減らしてしまっていた。
//   候補が無くなれば貪欲はそこで止まる = 「作らない」という選択ができる。
inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= TMAX) return 0;
    int m = 0;
    for (int r = 0; r < R; r++)
        if (PROF[r] > MARGIN && s.stock[RA[r]] >= RX[r] && s.stock[RB[r]] >= RY[r]
            && !(RA[r] == RB[r] && s.stock[RA[r]] < RX[r] + RY[r]))
            out[m++].rec = (int16_t)r;
    return m;
}
inline float calc_score(const State &s, const Move &mv) {
    (void)s;
    const int r = mv.rec;
    return (float)VALI[RC[r]] - (float)RX[r] * (float)VALI[RA[r]] - (float)RY[r] * (float)VALI[RB[r]];
}
// 評価値: 「材料 1 個あたりの見込み利益」。材料 (レベル 0 の在庫) が一番の制約なので、
//         利益の絶対額よりコスパで選んだ方が全体の取り分が増える (EFF で効かせ方を調整)
inline float eval_move(const State &s, const Move &mv) { (void)s; return EFFV[mv.rec]; }
inline void apply_move(State &s, const Move &mv) {
    const int r = mv.rec;
    s.stock[RA[r]] -= RX[r];
    s.stock[RB[r]] -= RY[r];
    s.stock[RC[r]] += 1;
    if (s.turn < MAXOP) s.ops[s.turn] = (int16_t)r;
    s.turn++;
}

// BONUS / PROF を作る。上のレベルから順に「1 単位あたりいくら上乗せできるか」を伝播させる
static void build_potential() {
    int maxlv = 0;
    for (int i = 0; i < M; i++) maxlv = max(maxlv, LV[i]);
    for (int i = 0; i < M; i++) BONUS[i] = 0.0f;
    for (int d = maxlv - 1; d >= 0; d--)
        for (int r = 0; r < R; r++) {
            const int a = RA[r], b = RB[r], c = RC[r];
            if (LV[a] != d) continue;
            const float p = (float)VALI[c] + BONUS_W * BONUS[c]
                          - (float)RX[r] * (float)VALI[a] - (float)RY[r] * (float)VALI[b];
            if (p <= 0.0f) continue;
            if (a == b) { const float v = p / (float)(RX[r] + RY[r]); if (v > BONUS[a]) BONUS[a] = v; }
            else {
                float v = p / (float)RX[r]; if (v > BONUS[a]) BONUS[a] = v;
                v = p / (float)RY[r];       if (v > BONUS[b]) BONUS[b] = v;
            }
        }
    for (int r = 0; r < R; r++) {
        PROF[r] = (float)VALI[RC[r]] + BONUS_W * BONUS[RC[r]]
                - (float)RX[r] * (float)VALI[RA[r]] - (float)RY[r] * (float)VALI[RB[r]];
        EFFV[r] = PROF[r] / powf((float)(RX[r] + RY[r]), EFF);
    }
}

void read_input() {
    if (scanf("%d %d %d", &M, &R, &TMAX) == 3 && M >= 1) {
        M = min(M, MAXM); R = min(R, MAXR); TMAX = min(TMAX, MAXOP);
        for (int i = 0; i < M; i++) scanf("%d", &VALI[i]);
        for (int i = 0; i < M; i++) scanf("%d", &ST0[i]);
        for (int r = 0; r < R; r++) scanf("%d %d %d %d %d", &RA[r], &RX[r], &RB[r], &RY[r], &RC[r]);
        for (int i = 0; i < M; i++) LV[i] = i / max(1, M / 4);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 44);
        M = 40; R = 80; TMAX = 400;
        const int per = M / 4;
        for (int i = 0; i < M; i++) {
            LV[i] = i / per;
            VALI[i] = 1 + (int)g.next((uint32_t)(40 * (LV[i] + 1)));
            ST0[i] = (LV[i] == 0) ? 60 : 0;
        }
        for (int r = 0; r < R; r++) {
            const int d = (int)g.next(3);
            RA[r] = d * per + (int)g.next((uint32_t)per);
            RB[r] = d * per + (int)g.next((uint32_t)per);
            RC[r] = (d + 1) * per + (int)g.next((uint32_t)per);
            RX[r] = 1 + (int)g.next(3);
            RY[r] = 1 + (int)g.next(3);
        }
    }
    build_potential();
}

void output(const State &s) {
    string res; res.reserve((size_t)s.turn * 4);
    for (int t = 0; t < s.turn && t < MAXOP; t++) { res += to_string((int)s.ops[t]); res += '\n'; }
    fputs(res.c_str(), stdout);
}

float replay_true_score(const State &s) {
    static int st[MAXM];
    for (int i = 0; i < M; i++) st[i] = ST0[i];
    const int n = min((int)s.turn, MAXOP);
    if (n > TMAX) { fprintf(stderr, "[error] 操作回数超過\n"); return -1.0f; }
    for (int t = 0; t < n; t++) {
        const int r = s.ops[t];
        if (r < 0 || r >= R) { fprintf(stderr, "[error] 不正なレシピ\n"); return -1.0f; }
        st[RA[r]] -= RX[r]; st[RB[r]] -= RY[r];
        if (st[RA[r]] < 0 || st[RB[r]] < 0) { fprintf(stderr, "[error] 在庫不足 (op %d)\n", t); return -1.0f; }
        st[RC[r]] += 1;
    }
    float v = 0.0f;
    for (int i = 0; i < M; i++) v += (float)st[i] * (float)VALI[i];
    return v;
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
