// =============================================================================
//  [ビームサーチ系 09] 単語の連鎖 (しりとり最長化)
// =============================================================================
//  【問題】
//    N 個の単語が与えられる (アルファベットは a..d の 4 文字)。
//    単語を 1 個ずつ並べて連鎖を作る。ただし、直前の単語の最後の文字と
//    次の単語の最初の文字が一致していなければならない。各単語は 1 回までしか使えない。
//    連鎖に使った単語の長さの合計を最大化せよ。
//  【入力】
//    N
//    w_0 ... (N 行)
//  【出力】
//    使った単語の番号を、連鎖の順に 1 行に 1 つ出力する。
//  【スコア】 使った単語の長さの合計 (大きいほど良い)。連鎖条件を満たさなければ 0 点。
//  【入力生成方法】
//    N=50 固定。各単語は長さ 3..8 の一様整数長で、各文字は a..d の一様乱数。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「次にどの単語を繋ぐか」の逐次決定なので、状態を (使用済みビット, 末尾の文字) にすれば
//     そのままビームサーチになる。単語 50 個なので使用済みは 64bit 整数 1 本。
//     ここで効くのが対称性の除去。
//     **同じ (先頭文字, 末尾文字) を持つ単語どうしは、連鎖の中で完全に入れ替え可能**。
//     入れ替えても連鎖はつながったままで、長い方を使った方が損しない。
//     だから「そのグループでは長い方から順に使う」と決めてよく、
//     1 手の候補は「次の文字」の数 (= 4) だけになる。
//     同じ本数を使った状態は used が完全に一致するので、ハッシュで潰し切れる。
//
//   ● 状態 (State)
//     使用済みビット、末尾の文字、ターン。
//
//   ● 手の作り方
//     末尾の文字で始まるグループごとに「まだ使っていない中で一番長い単語」1 本ずつ。最大 4 通り。
//
//   ● 評価値
//     使った長さ + EVAL_W × (残りで取れる長さの上界)
//     上界の作り方はオイラー路の数え方と同じ。
//     各文字から出られる回数は min(その文字で始まる残り数, その文字で終わる残り数 (+今いる文字なら 1))
//     で抑えられるので、そこから本数 M を見積もり、残っている単語を長い方から M 本ぶん足す。
//     この項は**差分で入れる** (ポテンシャル整形) ので、行き止まりでは 0 になり、
//     完成解の評価値は真のスコアとぴったり一致する。
//
//   ● 差分計算 / 高速化
//     グループごとの残り本数を配列で持てば、上界の計算は文字数 (4) のループで済む。
//     ハッシュは used と末尾文字から作る。
//
//   ● つまずきポイント
//     ・対称性の除去を入れないと、同じ「使った単語の集合」が並べ替え違いで大量に散らばり、
//       ビームが実質的に使えない。
//     ・上界を絶対値で足すと終端の比較が狂う。差分で入れること。
//
//   ● さらに伸ばすなら
//     ・別に書いた厳密解 (グループ本数ベクトルを memo 化した DFS) と照合したところ、
//       seed 0-19 のすべてで最適値に一致した。この設定では上限。
//     ・伸ばすならアルファベットを増やす、単語数を増やす方向
//
//  【改善】
//    (1) 同じ (先頭文字, 末尾文字) のグループは長い順に使うと決めて候補を 50 通り -> 4 通りに削減。
//    (2) 先読みを「次に繋げる単語数」から「残りで取れる長さの上界」に差し替え、差分で加算する形にした。
//    別に書いた厳密解 (グループ本数ベクトルを memo 化した DFS) と照合したところ、
//    seed 0..19 の全てで最適値に一致した (変更前は seed 5 と 9 で最適値に届いていなかった)。
//    seed 0..4 合計: 変更前 1248 -> 変更後 1248 (+0.0%  ※変更前も既に最適値だった)
//    seed 5..9 合計: 変更前 1272 -> 変更後 1275 (+0.24%) / seed 10..19 は 2422 で同点
//
//  【採用したライブラリ】 beam search/3_chokudai_search
//    実測比較 (seed 0,1,2 の合計スコア): 1_beam_search=744 / 3_chokudai_search=749
// =============================================================================
// =============================================================================
//  chokudai サーチ (Chokudai Search)   ---  AHC 用 高速テンプレート
// =============================================================================
//  深さごとの優先度付きキューを何周も回す。時間を使い切れて多様性も出る anytime 探索。
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
int   CHOKUDAI_WIDTH = 1;    // p1: 1 周で各深さから展開する数
float EVAL_W         = 1.0f;   // p2: 先読み項 (残りで取れる長さの上界) の重み

void load_params() {
    pick_env("p1", CHOKUDAI_WIDTH);
    pick_env("p2", EVAL_W);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化
// constexpr bool MAXIMIZE = false;    // ← スコア最小化

// a が b より良ければ true
inline bool is_better(float a, float b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }
// 最悪値 (初期化用)
constexpr float WORST_SCORE = MAXIMIZE ? -3.0e38f : 3.0e38f;

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS  = 1900.0f;  // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr int  POOL_CAP       = 1 << 17;  // ★状態プールの大きさ (State のサイズ × これだけメモリを使う)
constexpr int  HIST_CAP       = 1 << 22;  // 生きている履歴ノードの上限 (超えたら回収する)
constexpr bool USE_HASH_DEDUP = true;     // 同じ状態を二度展開しない (深さ込みハッシュ)

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_expanded = 0, g_cand_total = 0;
static int g_turn_done = 0;
static float g_best_eval = 0.0f;
static ll g_sweeps = 0, g_gc = 0;
// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (単語の連鎖)
// #############################################################################
constexpr int MAXN = 60, MAXL = 16;
constexpr int MAX_BRANCH = MAXN;
constexpr int BEAM_CAP   = 8192;

constexpr int MAXA = 4;                       // 想定するアルファベットの種類数 (問題より a..d)
constexpr int MAXG = MAXA * MAXA;

int N, MAX_TURN;
static char WD[MAXN][MAXL];
static int  WL[MAXN], WF[MAXN], WB[MAXN];

// ---- 前計算 ----------------------------------------------------------------
//  同じ (先頭文字, 末尾文字) の単語どうしは連鎖の中で完全に入れ替え可能なので、
//  「そのグループで長い方から順に使う」と決めてしまってよい (交換して損しない)。
//  こうすると 1 手の候補が「次の文字 4 通り」だけになり、
//  さらに同じ本数を使った状態は used が一致するのでハッシュで完全に潰せる。
static int      ALPHA = MAXA;
static bool     CANON = true;                 // 前提が崩れる入力なら false (従来の全列挙に戻す)
static int      GN[MAXG];                     // グループの単語数
static int      GRP[MAXG][MAXN];              // グループの単語を長い順に並べたもの
static uint64_t GMASK[MAXG];                  // グループの単語のビット集合
static uint64_t OUTMASK[MAXA], INMASK[MAXA];  // その文字で始まる / 終わる単語の集合
static int      OUTN[MAXA], INN[MAXA];
static uint64_t LENMASK[MAXL];                // 長さごとの単語の集合
static int      LENCNT[MAXL], MINWL = 1, MAXWL = 1;

struct Move { int8_t id; };

struct State {
    float    score;
    float    pot;           // 残りで取れる長さの上界 (評価の先読み項)
    uint64_t hash;
    uint64_t used;
    int8_t   last;          // 末尾の文字 (-1 は開始前)
    int32_t  turn;
};

// ---- 残りで取れる長さの上界 (先読み項) --------------------------------------
//  各文字 v から出られる回数は「v で始まる未使用語の数」と「v で終わる未使用語の数(+開始点なら 1)」の
//  小さい方で頭打ちになる (オイラー路と同じ数え方)。その合計 M 本ぶん、
//  残っている単語を長い方から取ったときの長さの合計が上界になる。
static inline float potential(uint64_t used, int last) {
    if (last >= 0 && OUTN[last] - (int)__builtin_popcountll(used & OUTMASK[last]) == 0) return 0.0f;  // 行き止まり
    int M = 0;
    for (int v = 0; v < ALPHA; v++) {
        const int o = OUTN[v] - (int)__builtin_popcountll(used & OUTMASK[v]);
        const int i = INN[v]  - (int)__builtin_popcountll(used & INMASK[v]) + (v == last ? 1 : 0);
        M += o < i ? o : i;
    }
    float sum = 0.0f;
    for (int L = MAXWL; L >= MINWL && M > 0; L--) {
        int c = LENCNT[L] - (int)__builtin_popcountll(used & LENMASK[L]);
        if (c > M) c = M;
        sum += (float)(L * c);
        M -= c;
    }
    return sum;
}

static void precompute() {
    ALPHA = 1;
    for (int i = 0; i < N; i++) { if (WF[i] + 1 > ALPHA) ALPHA = WF[i] + 1; if (WB[i] + 1 > ALPHA) ALPHA = WB[i] + 1; }
    CANON = (ALPHA <= MAXA && N <= 64);
    if (ALPHA > MAXA) ALPHA = MAXA;
    for (int g = 0; g < MAXG; g++) { GN[g] = 0; GMASK[g] = 0; }
    for (int v = 0; v < MAXA; v++) { OUTMASK[v] = INMASK[v] = 0; OUTN[v] = INN[v] = 0; }
    for (int l = 0; l < MAXL; l++) { LENMASK[l] = 0; LENCNT[l] = 0; }
    MINWL = MAXL - 1; MAXWL = 1;
    if (!CANON) return;
    for (int v = 0; v < ALPHA; v++) {
        for (int b = 0; b < ALPHA; b++) {
            const int g = v * ALPHA + b;
            for (int i = 0; i < N; i++) if (WF[i] == v && WB[i] == b) GRP[g][GN[g]++] = i;
            sort(GRP[g], GRP[g] + GN[g], [](int a, int b2) { return WL[a] > WL[b2]; });   // 長い順
            for (int k = 0; k < GN[g]; k++) GMASK[g] |= 1ULL << GRP[g][k];
        }
    }
    for (int i = 0; i < N; i++) {
        OUTMASK[WF[i]] |= 1ULL << i; OUTN[WF[i]]++;
        INMASK [WB[i]] |= 1ULL << i; INN [WB[i]]++;
        const int l = WL[i] < MAXL ? WL[i] : MAXL - 1;
        LENMASK[l] |= 1ULL << i; LENCNT[l]++;
        if (l < MINWL) MINWL = l;
        if (l > MAXWL) MAXWL = l;
    }
}

void init_state(State &s) {
    precompute();
    s.score = 0.0f; s.hash = 0; s.used = 0; s.last = -1; s.turn = 0;
    s.pot = 0.0f;   // ポテンシャル整形の基準を 0 にすると 評価値 = 真のスコア になる
}

inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    if (!CANON) {
        for (int i = 0; i < N; i++)
            if (!((s.used >> i) & 1) && (s.last < 0 || WF[i] == s.last)) out[m++].id = (int8_t)i;
        return m;
    }
    if (s.last < 0) {                       // 最初の 1 語だけは全グループが候補
        const int G = ALPHA * ALPHA;
        for (int g = 0; g < G; g++) {
            const int c = (int)__builtin_popcountll(s.used & GMASK[g]);
            if (c < GN[g]) out[m++].id = (int8_t)GRP[g][c];
        }
    } else {
        const int base = s.last * ALPHA;
        for (int b = 0; b < ALPHA; b++) {
            const int g = base + b;
            const int c = (int)__builtin_popcountll(s.used & GMASK[g]);
            if (c < GN[g]) out[m++].id = (int8_t)GRP[g][c];
        }
    }
    return m;
}
// 評価値 = 実際の長さ + EVAL_W x 残りで取れる長さの上界。
// 先読み項を「差」で入れる (ポテンシャル整形) ので、行き止まりまで進んだ状態では
// 上界が 0 になり、評価値がそのまま真のスコアと一致する。
inline float calc_score(const State &s, const Move &mv) {
    const int i = mv.id;
    if (!CANON) {
        int nxt = 0;
        for (int j = 0; j < N; j++) if (j != i && !((s.used >> j) & 1) && WF[j] == WB[i]) nxt++;
        return (float)WL[i] + EVAL_W * (float)nxt;
    }
    return (float)WL[i] + EVAL_W * (potential(s.used | (1ULL << i), WB[i]) - s.pot);
}
inline uint64_t calc_hash(const State &s, const Move &mv) {
    const uint64_t u = s.used | (1ULL << mv.id);
    return (u * 0x9E3779B97F4A7C15ULL) ^ ((uint64_t)(WB[mv.id] + 1) * 0xC2B2AE3D27D4EB4FULL);
}
inline void apply_move(State &s, const Move &mv) {
    s.used |= 1ULL << mv.id;
    s.last = (int8_t)WB[mv.id];
    s.turn++;
    s.hash = (s.used * 0x9E3779B97F4A7C15ULL) ^ ((uint64_t)(s.last + 1) * 0xC2B2AE3D27D4EB4FULL);
    if (CANON) s.pot = potential(s.used, s.last);
}

void read_input() {
    if (scanf("%d", &N) == 1 && N >= 1) {
        N = min(N, MAXN);
        for (int i = 0; i < N; i++) scanf("%s", WD[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 309);
        N = 50;
        for (int i = 0; i < N; i++) {
            const int L = 3 + (int)g.next(6);
            for (int k = 0; k < L; k++) WD[i][k] = (char)('a' + g.next(4));
            WD[i][L] = 0;
        }
    }
    for (int i = 0; i < N; i++) { WL[i] = (int)strlen(WD[i]); WF[i] = WD[i][0] - 'a'; WB[i] = WD[i][WL[i] - 1] - 'a'; }
    MAX_TURN = N;
}

void output(const vector<Move> &path) {
    string r; r.reserve(path.size() * 4);
    for (const Move &mv : path) { r += to_string((int)mv.id); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &path) {
    uint64_t used = 0; int last = -1; float tot = 0.0f;
    for (const Move &mv : path) {
        const int i = mv.id;
        if (i < 0 || i >= N || ((used >> i) & 1)) { fprintf(stderr, "[error] 単語の重複使用\n"); return -1.0f; }
        if (last >= 0 && WF[i] != last) { fprintf(stderr, "[error] 連鎖条件を満たさない\n"); return -1.0f; }
        used |= 1ULL << i; last = WB[i]; tot += (float)WL[i];
    }
    return tot;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

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
// 7. chokudai サーチ 本体
// =============================================================================
//  深さごとに優先度付きキューを持ち、「深さ 0,1,2,... から 1 個ずつ取り出して展開」を
//  1 周として、時間がある限り何周も回す。
//  ・ビームサーチと違い、時間を余らせない/使い切れる (anytime アルゴリズム)
//  ・毎周ちがう解が選ばれるので多様性が出る (ビーム幅 1 のビームサーチの繰り返しに近い)
//  ・キューは (スコア, プール添字) だけを持つので、ヒープ操作で State をコピーしない
struct Entry  { float score; int32_t idx; };
// std::push_heap / pop_heap で「一番良いものが top」になる比較
struct BestTop { bool operator()(const Entry &a, const Entry &b) const { return is_better(b.score, a.score); } };

static vector<vector<Entry>> g_pq;         // 深さごとのヒープ
static vector<State>   g_pool;             // 状態プール (使い回す)
static vector<int32_t> g_pool_h;           // プール要素 → 履歴ノード
static vector<int32_t> g_free;             // 空きスロット
static int g_gc_keep = 128;

static inline int32_t pool_alloc() { if (g_free.empty()) return -1; int32_t i = g_free.back(); g_free.pop_back(); return i; }
// 状態を捨てる。履歴の参照も落とすので、誰からも辿れない履歴は自動で回収される。
static inline void pool_free(int32_t i) { hist_release(g_pool_h[i]); g_free.push_back(i); }

// プールが尽きたら各深さの上位 g_gc_keep 個だけ残して回収する
static void pool_gc() {
    for (auto &q : g_pq) {
        if ((int)q.size() <= g_gc_keep) continue;
        nth_element(q.begin(), q.begin() + g_gc_keep, q.end(),
                    [](const Entry &a, const Entry &b) { return is_better(a.score, b.score); });
        for (size_t i = (size_t)g_gc_keep; i < q.size(); i++) pool_free(q[i].idx);
        q.resize(g_gc_keep);
        make_heap(q.begin(), q.end(), BestTop());
    }
    g_gc++;
}

// ---- 「深さ + ハッシュ」で同じ状態を二度展開しないためのテーブル ----
constexpr int      CHT_BITS = 20;
constexpr int      CHT_SIZE = 1 << CHT_BITS;
constexpr uint32_t CHT_MASK = CHT_SIZE - 1;
static uint64_t g_cht[CHT_SIZE];
static int      g_cht_cnt = 0;
static bool     g_cht_off = false;
static inline bool first_visit(uint64_t key) {
    if (g_cht_off) return true;
    if (key == 0) key = 1;
    uint32_t slot = (uint32_t)((key * 0x9E3779B97F4A7C15ULL) >> (64 - CHT_BITS));
    while (g_cht[slot] && g_cht[slot] != key) slot = (slot + 1) & CHT_MASK;
    if (g_cht[slot]) return false;
    g_cht[slot] = key;
    if (++g_cht_cnt > CHT_SIZE / 2) g_cht_off = true;   // 詰まってきたら重複除去を諦める
    return true;
}

vector<Move> chokudai_search(float deadline_ms) {
    g_pool.resize(POOL_CAP); g_pool_h.resize(POOL_CAP);
    g_free.clear(); g_free.reserve(POOL_CAP);
    for (int i = POOL_CAP - 1; i >= 0; i--) g_free.push_back(i);
    g_pq.assign(MAX_TURN + 1, {});
    hist_init();
    g_gc_keep = max(16, POOL_CAP / (4 * (MAX_TURN + 1)));

    const int32_t root = pool_alloc();
    init_state(g_pool[root]);
    g_pool_h[root] = -1;
    g_pq[0].push_back({g_pool[root].score, root});

    float   best   = WORST_SCORE;
    int32_t best_h = -1;
    Move    mvbuf[MAX_BRANCH];
    bool    stop = false;

    // 1 周目は必ず最後まで回す (完成解を 1 つは確保するため)
    while (!stop) {
        for (int d = 0; d < MAX_TURN && !stop; d++) {
            if (g_sweeps > 0 && (d & 15) == 0 && timer.ms() >= deadline_ms) { stop = true; break; }
            for (int w = 0; w < CHOKUDAI_WIDTH; w++) {
                if (g_pq[d].empty()) break;
                // 一番良い状態を取り出す
                pop_heap(g_pq[d].begin(), g_pq[d].end(), BestTop());
                const Entry e = g_pq[d].back(); g_pq[d].pop_back();
                const State  &s  = g_pool[e.idx];
                const int32_t sh = g_pool_h[e.idx];

                const int m = enum_moves(s, mvbuf);
                g_expanded++; g_cand_total += m;
                if (m == 0) {                                // ★終端に到達: 完成解として覚えておく
                    if (is_better(s.score, best)) {
                        best = s.score;
                        hist_release(best_h);
                        best_h = sh;
                        if (sh >= 0) g_hist[sh].rc++;
                    }
                    pool_free(e.idx);
                    continue;
                }
                for (int k = 0; k < m; k++) {
                    const float sc = s.score + calc_score(s, mvbuf[k]);
                    if constexpr (USE_HASH_DEDUP) {
                        const uint64_t h = calc_hash(s, mvbuf[k]) ^ ((uint64_t)(d + 1) * 0x9E3779B97F4A7C15ULL);
                        if (!first_visit(h)) continue;
                    }
                    if (d + 1 == MAX_TURN) {                     // 完成解に到達
                        if (is_better(sc, best)) {
                            best = sc;
                            hist_release(best_h);                // 古い最良解の履歴を解放
                            best_h = hist_new(sh, mvbuf[k]);
                        }
                        continue;                                // 完成解は展開しないので状態は要らない
                    }
                    if (g_hist_live >= HIST_CAP) pool_gc();      // 履歴が膨らんだら回収
                    int32_t ni = pool_alloc();
                    if (ni < 0) { pool_gc(); ni = pool_alloc(); if (ni < 0) { stop = true; break; } }
                    g_pool[ni] = s;                              // ★State のコピー
                    apply_move(g_pool[ni], mvbuf[k]);
                    g_pool[ni].score = sc;
                    g_pool_h[ni] = hist_new(sh, mvbuf[k]);
                    g_pq[d + 1].push_back({sc, ni});
                    push_heap(g_pq[d + 1].begin(), g_pq[d + 1].end(), BestTop());
                }
                pool_free(e.idx);                                // 展開済みの状態は回収
                if (stop) break;
            }
        }
        g_sweeps++;
        if (timer.ms() >= deadline_ms) break;
    }
    g_turn_done = MAX_TURN;
    g_best_eval = best;

    // ---- 履歴を根まで辿って手順を復元する ----
    return (best_h >= 0) ? hist_path(best_h) : vector<Move>();
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

    vector<Move> path = chokudai_search(TIME_LIMIT_MS);
    output(path);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)replay_true_score(path));
    fprintf(stderr, "turn = %d/%d, expanded = %lld, cand = %lld, time = %.1f ms\n",
            g_turn_done, MAX_TURN, g_expanded, g_cand_total, (double)timer.ms());
    fprintf(stderr, "sweeps = %lld, gc = %lld, hist_live = %d\n", g_sweeps, g_gc, g_hist_live);
    // ★評価値と真のスコアの対応を確認する (評価値 = 真のスコアにしている場合、ずれていたらバグ)
    fprintf(stderr, "[check] eval = %.3f (探索が最大化した評価値) / true = %.3f (手順を再生した真のスコア)\n",
            (double)g_best_eval, (double)replay_true_score(path));
    return 0;
}
