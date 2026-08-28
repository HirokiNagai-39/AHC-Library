// =============================================================================
//  [ビームサーチ系 02] ブロック積み (最大高さの最小化)
// =============================================================================
//  【問題】
//    幅 W の床がある (各列の初期高さは 0)。T 個のブロックが決まった順番で降ってくる。
//    ブロック t は幅 w_t、高さ h_t で、連続する w_t 列のどこかに置く。
//    置くと、その w_t 列の高さはすべて (その範囲の現在の最大高さ + h_t) になる。
//    T 個すべて置いた後の最大高さを最小化せよ。
//  【入力】
//    W T
//    w_t h_t   (T 行)
//  【出力】
//    T 行。ブロック t を置く左端の列番号。
//  【スコア】 最終的な最大高さ (小さいほど良い)。範囲外に置いたら 0 点。
//  【入力生成方法】
//    W=20, T=200 固定。w_t は 1..3 の一様整数、h_t は 1..3 の一様整数。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「次に何が来るか分かっているオンライン積み木」なので、1 手ずつ確定していくビームサーチが素直。
//     難しいのは評価値。最終的に効くのは最大高さだけだが、途中の最大高さを見ても
//     「まだ低い所がたくさんある」状況では差がつかず、探索が迷子になる。
//     そこで「面積から決まる下界」を使う。
//
//   ● 状態 (State)
//     各列の高さ配列 h[] と、その合計 Σh、現在の最大高さ、ターン。
//
//   ● 手の作り方
//     そのターンのブロック (幅 w) を置ける左端の位置 (W - w + 1 通り) が候補。
//
//   ● 評価値
//     max(現在の最大高さ, (Σh + これから降る総面積) / W) + EVAL_W × (高さの凸凹)
//     ・Σh は「置いた面積 + 隙間として捨てた面積」なので、
//       (Σh + 残り面積) / W は最終的な最大高さの下界になる。
//       途中では「隙間を作らない」という強い誘導になり、
//       最終ターンでは残り面積が 0 かつ Σh/W <= 最大高さ なので、ちょうど真のスコアと一致する。
//       ここでもポテンシャル型の評価になっている。
//     ・凸凹 (隣接列の高さの差の合計) は同点の並びを整えるための補助項。
//
//   ● 差分計算 / 高速化
//     ブロックを置くと変わるのは w 列ぶんだけなので、Σh と凸凹の差分は O(w)。
//     ハッシュは (列, 高さ) の Zobrist を持ち、高さが変わった列だけ XOR し直す。
//     以前は毎回 O(W) の配列コピーと全走査をしていた。
//
//   ● つまずきポイント
//     ・calc_hash が返す値と apply_move 後の state.hash は完全に一致させること。
//       ここに「ターン番号」を混ぜるかどうかで食い違いが起きやすい ([verify] が検出する)。
//     ・評価値の下界項を入れないと、序盤に平らに積むだけで最大高さが同点になり、探索が効かない。
//
//   ● さらに伸ばすなら
//     ・現在のスコアは ceil(総面積 / W) の合計 (= 理論的な下界) と一致しており、これ以上は縮まない
//       (seed 0-9 のすべてで下界に到達)。
//     ・伸ばすならブロックの形を複雑にする (凹型を入れる) など、問題側を難しくする方向
//
//  【改善】
//    評価値を「最大高さ + 凸凹」から「max(最大高さ, 面積から決まる下界) + EVAL_W × 凸凹」に変更。
//    無駄面積と凸凹の差分を O(w) で求めるようにして (以前は毎回 O(W) の配列コピー + 全走査)、
//    BEAM_WIDTH を 3000 -> 12000、BEAM_CAP を 16384 -> 32768 に拡大。
//    seed 0..4 合計: 変更前 202 -> 変更後 201 (-0.5%, 小さいほど良い)
//    ※ 201 は ceil(総面積/W) の合計 (= 理論的な下界) と一致しており、これ以上は縮まない。
//       seed 5..9 でも 200 -> 198 で下界に到達。
//
//  【採用したライブラリ】 beam search/1_beam_search
//    実測比較 (seed 0,1,2 の合計スコア): 1_beam_search=121 / 3_chokudai_search=125
// =============================================================================
// =============================================================================
//  ビームサーチ (Beam Search)   ---  AHC 用 高速テンプレート
// =============================================================================
//  各ターンで候補を全部作って上位 W 個だけ残す。State をコピーして次のビームを作る素直な実装。
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct Move  { ... };                    ... 遷移(手)。小さいほど速い
//    struct State { float score; uint64_t hash; ... };  ... 状態 (score は必須)
//    init_state(State&)                 ... 初期状態 (score, hash もセット)
//    enum_moves(const State&, Move*)    ... 遷移候補を全列挙して個数を返す
//    calc_score(const State&, Move)     ... その手を指したときの「スコア差分」
//    calc_hash (const State&, Move)     ... その手を指したときのハッシュ (重複除去用)
//    apply_move(State&, Move)           ... 手を実際に適用する (score 以外を更新)
//
//  【フレームワークとの約束】
//    ・子のスコア = 親の score + calc_score(親, 手)   ← score はフレームワークが更新する
//    ・apply_move は score を触らない (hash は自分で更新する)
//    ・State::score は「評価値」。真のスコアと違う評価にしても良い(デモはそうしている)
//
//  【高速化のポイント】
//    ・上位 W 個の選抜は全ソートせず nth_element (O(候補数))
//    ・重複除去はオープンアドレス法 + 世代スタンプ (クリア不要)
//    ・候補配列・状態バッファは全て静的確保。探索中に malloc を一切呼ばない
//    ・乱数は xorshift128、時間計測は rdtsc/cntvct 直読み
//    ・ビーム幅は残り時間から自動調整 (時間を使い切りつつ時間超過しない)
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
    inline uint64_t next64() { return ((uint64_t)next() << 32) | next(); }
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
        ms_per_tick_ = 1000.0 / (double)f; calib_ = 100;   // ARM は周波数が正確に取れる
#elif defined(__x86_64__) || defined(_M_X64)
        ms_per_tick_ = 1.0 / 2.8e6; calib_ = 0;            // 仮値(2.8GHz)。下で自動補正する
#else
        ms_per_tick_ = 1e-6; calib_ = 100;
#endif
        w0_ = chrono::steady_clock::now();
        t0_ = tick();
    }
    inline float ms() {
        uint64_t d = tick() - t0_;
        if (calib_ < 4) calibrate(d);       // 最初の数回だけ実時計で周波数を補正
        return (float)((double)d * ms_per_tick_);
    }
    void calibrate(uint64_t d) {
        double w = chrono::duration<double, milli>(chrono::steady_clock::now() - w0_).count();
        if (w > 0.25 * (double)(1u << (2 * calib_))) {
            ms_per_tick_ = w / (double)d;
            calib_++;
        }
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
int   BEAM_WIDTH = 12000;  // p1: ビーム幅 (ADAPTIVE_WIDTH=true なら基準値)
float EVAL_W     = 0.4f;   // p2: 高さの凸凹を嫌う重み
float P_SUM      = 0.0f;   // p3: 高さの合計 (= 捨てた面積) を嫌う重み

void load_params() {
    pick_env("p1", BEAM_WIDTH);
    pick_env("p2", EVAL_W);
    pick_env("p3", P_SUM);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
// constexpr bool MAXIMIZE = true;     // ← スコア最大化
constexpr bool MAXIMIZE = false;       // ← スコア最小化

// a が b より良ければ true
inline bool is_better(float a, float b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }
// 最悪値 (初期化用)
constexpr float WORST_SCORE = MAXIMIZE ? -3.0e38f : 3.0e38f;

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS  = 1900.0f;  // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr int  MIN_BEAM_WIDTH = 1;      // ビーム幅の下限
constexpr bool ADAPTIVE_WIDTH = true;   // 残り時間からビーム幅を自動調整する (時間を使い切る)
constexpr bool USE_HASH_DEDUP = true;   // ハッシュによる重複除去を行う

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_expanded = 0, g_cand_total = 0;
static int g_turn_done = 0;
static float g_best_eval = 0.0f;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (ブロック積み)
// #############################################################################
constexpr int MAXW = 32, MAXTT = 512, MAXHH = 1024;
constexpr int MAX_BRANCH = MAXW;
constexpr int BEAM_CAP   = 32768;             // ★ビーム幅の上限

int W, MAX_TURN;
static int BLW[MAXTT], BLH[MAXTT];
static int REM_AREA[MAXTT + 1];               // t 手目以降に降ってくるブロックの総面積
static uint64_t ZOB[MAXW][MAXHH];

struct Move { int8_t col; };

struct State {
    float    score;
    uint64_t hash;
    int32_t  h[MAXW];
    int32_t  turn;
    int32_t  mx;             // 現在の最大高さ
    int32_t  sum;            // 高さの合計 (= 置いた面積 + 無駄にした面積)
    int32_t  rough;          // 隣接列の高さ差の合計
};

// ---- 評価値 ----
//   Σh は「置いた面積 + 隙間として捨てた面積」なので、これから降る面積を足して幅で割ると
//   「最終的な最大高さの下界」になる。max(現在の最大高さ, その下界) は必ず最終スコア以下で、
//   最終ターンでは残り面積 0 かつ Σh/W <= mx なので、ちょうど最終スコアと一致する。
//   つまり途中では強い誘導になり、最後は真のスコアで順位が決まる (ポテンシャル型)。
static inline float eval_of(int mx, int sum, int turn) {
    const float lb = ((float)sum + (float)REM_AREA[turn]) * (1.0f / (float)W);
    return (mx > lb ? (float)mx : lb) + P_SUM * (float)sum * (1.0f / (float)W);
}

void init_state(State &s) {
    s.turn = 0; s.mx = 0; s.sum = 0; s.rough = 0; s.hash = 0;
    for (int i = 0; i < W; i++) { s.h[i] = 0; s.hash ^= ZOB[i][0]; }
    s.score = eval_of(0, 0, 0);
}

inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= MAX_TURN) return 0;
    const int w = BLW[s.turn];
    int m = 0;
    for (int c = 0; c + w <= W; c++) out[m++].col = (int8_t)c;
    return m;
}

static inline int base_of(const State &s, int c, int w) {
    int b = s.h[c];
    for (int k = 1; k < w; k++) if (s.h[c + k] > b) b = s.h[c + k];
    return b;
}

// 置いたことによる (無駄面積, 凸凹の増減) を O(w) で求める
static inline void delta_of(const State &s, int c, int w, int nh, int &waste, int &drough) {
    int ws = 0;
    for (int k = 0; k < w; k++) ws += nh - BLH[s.turn] - s.h[c + k];
    waste = ws;
    int o = 0, n = 0;
    if (c > 0)         { o += abs(s.h[c - 1] - s.h[c]);         n += abs(s.h[c - 1] - nh); }
    for (int k = 0; k + 1 < w; k++) o += abs(s.h[c + k] - s.h[c + k + 1]);
    if (c + w < W)     { o += abs(s.h[c + w - 1] - s.h[c + w]); n += abs(nh - s.h[c + w]); }
    drough = n - o;
}

inline float calc_score(const State &s, const Move &mv) {
    const int w = BLW[s.turn], nh = base_of(s, mv.col, w) + BLH[s.turn];
    const int nmx = max(s.mx, nh);
    int waste, drough;
    delta_of(s, mv.col, w, nh, waste, drough);
    const int nsum = s.sum + w * BLH[s.turn] + waste;
    return eval_of(nmx, nsum, s.turn + 1) - eval_of(s.mx, s.sum, s.turn)
         + EVAL_W * (float)drough;
}

inline uint64_t calc_hash(const State &s, const Move &mv) {
    const int w = BLW[s.turn], nh = min(MAXHH - 1, base_of(s, mv.col, w) + BLH[s.turn]);
    uint64_t h = s.hash;
    for (int k = 0; k < w; k++) h ^= ZOB[mv.col + k][min(MAXHH - 1, s.h[mv.col + k])] ^ ZOB[mv.col + k][nh];
    return h;   // ★apply_move 後の s.hash と完全に一致させること (ターン係数は入れない)
}

inline void apply_move(State &s, const Move &mv) {
    const int w = BLW[s.turn], nh = base_of(s, mv.col, w) + BLH[s.turn];
    int waste, drough;
    delta_of(s, mv.col, w, nh, waste, drough);
    s.sum += w * BLH[s.turn] + waste;
    s.rough += drough;
    for (int k = 0; k < w; k++) {
        s.hash ^= ZOB[mv.col + k][min(MAXHH - 1, s.h[mv.col + k])] ^ ZOB[mv.col + k][min(MAXHH - 1, nh)];
        s.h[mv.col + k] = nh;
    }
    s.mx = max(s.mx, nh);
    s.turn++;
}

void read_input() {
    if (scanf("%d %d", &W, &MAX_TURN) == 2 && W >= 1) {
        W = min(W, MAXW); MAX_TURN = min(MAX_TURN, MAXTT);
        for (int t = 0; t < MAX_TURN; t++) scanf("%d %d", &BLW[t], &BLH[t]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 302);
        W = 20; MAX_TURN = 200;
        for (int t = 0; t < MAX_TURN; t++) { BLW[t] = 1 + (int)g.next(3); BLH[t] = 1 + (int)g.next(3); }
    }
    REM_AREA[MAX_TURN] = 0;
    for (int t = MAX_TURN - 1; t >= 0; t--) REM_AREA[t] = REM_AREA[t + 1] + BLW[t] * BLH[t];
    Xor128 z; z.seed(555);
    for (int i = 0; i < W; i++) for (int hh = 0; hh < MAXHH; hh++) ZOB[i][hh] = ((uint64_t)z.next() << 32) | z.next();
}

void output(const vector<Move> &path) {
    string r; r.reserve(path.size() * 4);
    for (const Move &mv : path) { r += to_string((int)mv.col); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &path) {
    static int32_t h[MAXW];
    memset(h, 0, sizeof(int32_t) * (size_t)W);
    int mx = 0;
    for (int t = 0; t < (int)path.size(); t++) {
        const int c = path[t].col, w = BLW[t];
        if (c < 0 || c + w > W) { fprintf(stderr, "[error] 範囲外 (t=%d)\n", t); return -1.0f; }
        int b = 0;
        for (int k = 0; k < w; k++) b = max(b, h[c + k]);
        const int nh = b + BLH[t];
        for (int k = 0; k < w; k++) h[c + k] = nh;
        mx = max(mx, nh);
    }
    if ((int)path.size() != MAX_TURN) { fprintf(stderr, "[error] 手数が足りない\n"); return -1.0f; }
    return (float)mx;
}
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
//  候補と、ハッシュによる重複除去テーブル
// =============================================================================
// 候補 1 個 = 24 バイト。cache に載せるため詰めて持つ。
struct Cand {
    uint64_t hash;
    float    score;      // 親の score + calc_score() (絶対値)
    int32_t  parent;     // 親のビーム内添字
    Move     mv;
};

// オープンアドレス法 + 世代スタンプ。毎ターン clear しなくて良いので速い。
// テーブルは候補数の 2 倍以上を確保する (詰まると線形探索が遅くなるため)。
constexpr int ht_bits_for(int n) { int b = 1; while ((1 << b) < n) b++; return b; }
constexpr int      HT_BITS = ht_bits_for(2 * BEAM_CAP * MAX_BRANCH);
constexpr int      HT_SIZE = 1 << HT_BITS;
constexpr uint32_t HT_MASK = HT_SIZE - 1;
static uint64_t ht_key[HT_SIZE];
static int32_t  ht_val[HT_SIZE];
static uint32_t ht_gen_of[HT_SIZE];
static uint32_t ht_gen = 0;

// 同じハッシュの候補を「良い方だけ」に潰して詰め直し、残った個数を返す O(n)
static int dedup(Cand *c, int n) {
    ++ht_gen;
    int m = 0;
    for (int i = 0; i < n; i++) {
        const uint64_t h = c[i].hash;
        uint32_t slot = (uint32_t)((h * 0x9E3779B97F4A7C15ULL) >> (64 - HT_BITS));
        while (ht_gen_of[slot] == ht_gen && ht_key[slot] != h) slot = (slot + 1) & HT_MASK;
        if (ht_gen_of[slot] != ht_gen) {
            ht_gen_of[slot] = ht_gen; ht_key[slot] = h; ht_val[slot] = m;
            c[m++] = c[i];
        } else {
            const int32_t j = ht_val[slot];
            if (is_better(c[i].score, c[j].score)) c[j] = c[i];
        }
    }
    return m;
}

// 上位 w 個を前に集める (全ソートしないので O(n))
static inline int select_top(Cand *c, int n, int w) {
    if (n <= w) return n;
    nth_element(c, c + w, c + n, [](const Cand &a, const Cand &b) { return is_better(a.score, b.score); });
    return w;
}

// =============================================================================
//  手順復元用の履歴 (参照カウント付き)
// =============================================================================
//  「今生きているビーム / キューから辿れる分」だけを保持するので、メモリが増え続けない。
//  死んだ枝はビームから外れた瞬間に自動で回収される。
//  (差分更新ビームサーチはツアーそのものが木なので、この仕組みは不要)
struct HNode { int32_t par; int32_t rc; Move mv; };
static vector<HNode>   g_hist;
static vector<int32_t> g_hfree;
static int g_hist_live = 0;

static void hist_init() { g_hist.clear(); g_hfree.clear(); g_hist_live = 0; g_hist.reserve(1 << 16); }

static inline int32_t hist_new(int32_t par, Move mv) {
    if (par >= 0) g_hist[par].rc++;
    g_hist_live++;
    if (!g_hfree.empty()) {
        const int32_t i = g_hfree.back(); g_hfree.pop_back();
        g_hist[i] = {par, 1, mv};
        return i;
    }
    g_hist.push_back({par, 1, mv});
    return (int32_t)g_hist.size() - 1;
}
static inline void hist_release(int32_t i) {
    while (i >= 0 && --g_hist[i].rc == 0) { g_hfree.push_back(i); g_hist_live--; i = g_hist[i].par; }
}
static vector<Move> hist_path(int32_t i) {
    vector<Move> p;
    for (; i >= 0; i = g_hist[i].par) p.push_back(g_hist[i].mv);
    reverse(p.begin(), p.end());
    return p;
}

// =============================================================================
// 7. ビームサーチ 本体
// =============================================================================
//  各ターンで「今のビームの全状態 × 全遷移」を候補にし、上位 BEAM_WIDTH 個を次のビームにする。
//  次のビームは State をコピーして作る（＝素直な実装）。State が小さいならこれが最速。
//  State が大きい場合はコピーが支配的になるので 2_incremental_beam_search を使うこと。
static State g_buf[2][BEAM_CAP];                  // 状態バッファ (ピンポンで使い回す)
static Cand  g_cand[BEAM_CAP * MAX_BRANCH];       // 候補 (静的確保。探索中は malloc しない)
static int32_t g_hidx[2][BEAM_CAP];               // ビーム各要素 → 履歴ノード
// 途中で手が無くなった状態 (終端解) のうち最良のものを保持しておく
static float   g_term_best = WORST_SCORE;
static int32_t g_term_hist = -1;

vector<Move> beam_search(float deadline_ms) {
    hist_init();

    int cur = 0, n_cur = 1;
    init_state(g_buf[0][0]);
    g_hidx[0][0] = -1;

    int   width   = min(BEAM_WIDTH, BEAM_CAP);
    float prev_ms = timer.ms();
    Move  mvbuf[MAX_BRANCH];
    int   turn = 0;

    for (; turn < MAX_TURN; turn++) {
        // ---- 1) 候補列挙 (ビームの全状態 × 全遷移) ----
        int nc = 0;
        for (int i = 0; i < n_cur; i++) {
            const State &s = g_buf[cur][i];
            const int m = enum_moves(s, mvbuf);
            if (m == 0) {                            // ★終端に到達: 完成解として覚えておく
                if (is_better(s.score, g_term_best)) {
                    g_term_best = s.score;
                    hist_release(g_term_hist);
                    g_term_hist = g_hidx[cur][i];
                    if (g_term_hist >= 0) g_hist[g_term_hist].rc++;
                }
                continue;
            }
            for (int k = 0; k < m; k++) {
                Cand &c = g_cand[nc++];
                c.hash   = calc_hash(s, mvbuf[k]);
                c.score  = s.score + calc_score(s, mvbuf[k]);   // 子のスコア = 親 + 差分
                c.parent = i;
                c.mv     = mvbuf[k];
            }
        }
        g_expanded += n_cur; g_cand_total += nc;
        if (nc == 0) break;                          // これ以上進めない

        // ---- 2) 重複除去 → 3) 上位 width 個を選抜 (どちらも O(候補数)) ----
        if constexpr (USE_HASH_DEDUP) nc = dedup(g_cand, nc);
        nc = select_top(g_cand, nc, width);

        // ---- 4) 次のビームを作る ----
        const int nxt = cur ^ 1;
        for (int i = 0; i < nc; i++) {
            const Cand &c = g_cand[i];
            State &d = g_buf[nxt][i];
            d = g_buf[cur][c.parent];                // ★State のコピー (ここが重い)
            apply_move(d, c.mv);
            d.score = c.score;
            g_hidx[nxt][i] = hist_new(g_hidx[cur][c.parent], c.mv);    // 手順復元用
        }
        for (int i = 0; i < n_cur; i++) hist_release(g_hidx[cur][i]);  // 旧ビームの参照を落とす
        const int processed = n_cur;
        cur = nxt; n_cur = nc;

        // ---- 5) 時間計測とビーム幅の調整 (1 ターンに 1 回だけ) ----
        const float now = timer.ms();
        if constexpr (ADAPTIVE_WIDTH) {
            const int rem = MAX_TURN - turn - 1;
            if (rem > 0) {
                const float per    = (now - prev_ms) / (float)max(1, processed);  // 1 状態あたりの時間
                const float budget = (deadline_ms - now) / (float)rem;            // 1 ターンに使える時間
                int nw = (int)(budget / max(per, 1e-9f));
                nw = min(nw, width * 2); nw = max(nw, width / 2);   // 1 ターンでの急変を抑える
                // BEAM_WIDTH を基準に [1/4 倍, 2 倍] の範囲で調整する
                width = min(nw, min(BEAM_WIDTH * 2, BEAM_CAP));
                width = max(width, max(MIN_BEAM_WIDTH, BEAM_WIDTH / 4));
            }
        }
        if (now >= deadline_ms) width = 1;           // 時間切れ: 幅 1 で最後まで走り切る
        prev_ms = now;
    }
    g_turn_done = turn;

    // ---- 最良の葉から手順を復元する ----
    int best = 0;
    for (int i = 1; i < n_cur; i++)
        if (is_better(g_buf[cur][i].score, g_buf[cur][best].score)) best = i;
    // 最終ターンまで残った解と、途中で終端に達した解の良い方を採用する
    if (n_cur == 0 || (g_term_hist >= 0 && is_better(g_term_best, g_buf[cur][best].score))) {
        g_best_eval = g_term_best;
        return hist_path(g_term_hist);
    }
    g_best_eval = g_buf[cur][best].score;
    return hist_path(g_hidx[cur][best]);
}

// =============================================================================
//  ★ハッシュと候補数の自動検証 (main の最初に走る)
// =============================================================================
//  ビームサーチの [check] は「探索の評価値」と「真のスコア」を比べているが、この 2 つは
//  そもそも一致しなくて良い (calc_score に先読み項を入れるのが普通)。
//  つまり [check] だけでは次のバグを検出できない。
//    ・calc_hash() が apply_move() 後の state.hash と食い違う
//      → 別物の状態を「重複」と誤判定して捨ててしまい、探索が静かに弱くなる
//    ・enum_moves() が MAX_BRANCH を超える数を書き込む → 配列外書き込み (最悪クラッシュ)
//  ここではその 2 つを 1 手ずつ突き合わせる。
//  ※ 乱数の状態は前後で復元するので、VERIFY_MOVES を変えても探索結果は 1 ビットも変わらない。
//  ※ 提出時に消したければ VERIFY_MOVES = 0 にする。
constexpr int VERIFY_MOVES = 300;

static void verify_state() {
    if constexpr (VERIFY_MOVES <= 0) return;
    const Xor128 save = rng;                       // 乱数列を汚さないよう退避
    static State s;
    static Move vbuf[MAX_BRANCH * 4 + 16];         // 溢れを検出するため多めに取る
    init_state(s);
    int bad = 0, over = 0;
    for (int i = 0; i < VERIFY_MOVES; i++) {
        const int m = enum_moves(s, vbuf);
        if (m > MAX_BRANCH) { over = m; break; }
        if (m == 0) { init_state(s); continue; }   // 終端まで来たら初期状態から
        const Move mv = vbuf[rng.next((uint32_t)m)];
        const uint64_t h = calc_hash(s, mv);
        const float d = calc_score(s, mv);
        apply_move(s, mv);
        s.score += d;
        if (s.hash != h && ++bad <= 5)
            fprintf(stderr, "[verify] NG %d 手目: calc_hash=%016llx / apply_move 後の hash=%016llx\n",
                    i, (unsigned long long)h, (unsigned long long)s.hash);
    }
    if (over)     fprintf(stderr, "[verify] ★enum_moves が %d 個返しました。MAX_BRANCH=%d を超えています (配列外書き込み)\n", over, MAX_BRANCH);
    else if (bad) fprintf(stderr, "[verify] ★calc_hash が %d/%d 手で apply_move 後の hash と食い違います (重複除去が壊れます)\n", bad, VERIFY_MOVES);
    else          fprintf(stderr, "[verify] ハッシュと候補数 OK (%d 手を検査)\n", VERIFY_MOVES);
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
    verify_state();     // ★ハッシュ / 候補数の自動検証 (提出時に不要なら VERIFY_MOVES = 0)

    vector<Move> path = beam_search(TIME_LIMIT_MS);
    output(path);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)replay_true_score(path));
    fprintf(stderr, "turn = %d/%d, expanded = %lld, cand = %lld, time = %.1f ms\n",
            g_turn_done, MAX_TURN, g_expanded, g_cand_total, (double)timer.ms());
    if (timer.ms() < TIME_LIMIT_MS * 0.8f)
        fprintf(stderr, "[hint] 時間が %.0f ms 余っています。BEAM_WIDTH (必要なら BEAM_CAP も) を大きくできます\n",
                (double)(TIME_LIMIT_MS - timer.ms()));
    // ★評価値と真のスコアの対応を確認する (評価値 = 真のスコアにしている場合、ずれていたらバグ)
    fprintf(stderr, "[check] eval = %.3f (探索が最大化した評価値) / true = %.3f (手順を再生した真のスコア)\n",
            (double)g_best_eval, (double)replay_true_score(path));
    return 0;
}
