// =============================================================================
//  [ビームサーチ系 05] 順列の整列 (操作コスト付き)
// =============================================================================
//  【問題】
//    長さ N の順列 p が与えられる。次の 2 種類の操作を最大 T 回まで行える。
//      (a) 隣り合う 2 要素を入れ替える           (コスト 1)
//      (b) 長さ L (2<=L<=6) の区間を反転する      (コスト L)
//    最終的な「転倒数 (i<j かつ p_i>p_j である組の数)」+ (支払ったコストの合計) を最小化せよ。
//  【入力】
//    N T
//    p_0 ... p_{N-1}
//  【出力】
//    実行した操作を 1 行に 1 つ "i j" (区間 [i,j] を反転。j=i+1 かつコスト 2 ではなく
//    隣接スワップとしたい場合も同じ形式で書き、コストは j-i+1 とする)。
//  【スコア】 最終転倒数 + 総コスト (小さいほど良い)。範囲外の操作があれば 0 点。
//  【入力生成方法】
//    N=60, T=150 固定。p は 0..N-1 のランダムな順列。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「操作を 1 つずつ選んでいく」逐次決定なのでビームサーチが自然。
//     ただし素朴に候補を出すと (開始位置 × 長さ 2..6) で 5N = 300 通りあり、1 手が重い。
//     ここで差分の式をよく見ると、区間 [i, i+L) を反転したときのスコア差分は
//       L(L-1)/2 - 2 × (区間内の転倒数) + L
//     で、L=2 なら最小 +1、L=3 なら最小 0。つまり**得をするのは L>=4 の「かなり降順な区間」だけ**。
//     そこを候補に絞ると 300 通りが 15 通り程度になり、同じ時間で実質のビーム幅が一桁大きくなる。
//
//   ● 状態 (State)
//     順列 p[] とターン。p[] は int8_t にしてコピーを半減させてある。
//
//   ● 手の作り方
//     「スコア差分が MOVE_TH (= 0) 以下になる反転」だけを候補にする。
//     候補が 1 つも無ければそこで終端 (それ以上動かしても損なので、打ち切って良い)。
//
//   ● 評価値
//     真のスコアの差分そのもの (転倒数の変化 + 支払いコスト)。
//     先読み項を足す実験もしたが、どれも改善しなかった。
//     この問題は「損な手を打たない」ことが本質なので、候補の絞り込みが先読みの役割を果たしている。
//
//   ● 差分計算 / 高速化
//     差分は enum_moves の中で 1 度だけ計算して Move に持たせるので、calc_score は O(1)。
//     区間内の転倒数は L を 1 ずつ伸ばしながら漸化式で求め、
//     「これ以上長くしても差分が 0 以下にならない」ところで打ち切る。
//     ハッシュは順列の Zobrist を、反転で位置が変わった要素だけ XOR し直す。
//
//   ● つまずきポイント
//     ・MOVE_TH を 1 や 2 に緩めて候補を増やすと、分岐が増えたぶんの損の方が大きく逆効果だった。
//     ・MAX_BRANCH を小さくできると BEAM_CAP を大きく取れる (静的配列は BEAM_CAP × MAX_BRANCH に比例)。
//       ここでは 300 -> 16 にできたので BEAM_CAP を 1024 -> 65536 まで広げられた。
//
//   ● さらに伸ばすなら
//     ・隣接スワップ (L=2) を別枠で少数だけ許して、最後の詰めをできるようにする
//     ・焼きなましに移して「操作列そのもの」を近傍で動かす方向
//
//  【改善】
//    (1) 候補を「スコア差分が 0 以下の反転」だけに絞り込み (MOVE_TH)。手が無くなれば終端扱い。
//    (2) 差分を Move に持たせて calc_score を O(1) 化、区間内転倒数は漸化で求め、
//        「これ以上長くしても差分が 0 以下にならない」ところで打ち切る (MINB)。
//    (3) State の順列を int16_t -> int8_t にしてコピーを半減。
//        MAX_BRANCH 300 -> 16 にできたので BEAM_CAP 1024 -> 65536 まで広げた。
//    seed 0..4 合計: 変更前 3185 -> 変更後 2899 (-9.0%)  ※ seed 0..9 では 6253 -> 5590 (-10.6%)
//
//  【採用したライブラリ】 beam search/1_beam_search
//    実測比較 (seed 0,1,2 の合計スコア): 1_beam_search=1889 / 3_chokudai_search=1916
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
int   BEAM_WIDTH = 32768;  // p1: ビーム幅 (ADAPTIVE_WIDTH=true なら基準値)
int   MOVE_TH    = 0;      // p2: スコア差分がこの値以下の反転だけを候補にする (候補の絞り込み)
float EVAL_W     = 0.0f;   // p3: (未使用。評価値 = 真のスコアのままが最良だった)
int   TOPK       = 16;     // p4: 1 状態あたりに残す候補数の上限 (差分の小さい順)

void load_params() {
    pick_env("p1", BEAM_WIDTH);
    pick_env("p2", MOVE_TH);
    pick_env("p3", EVAL_W);
    pick_env("p4", TOPK);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (順列の整列)
// #############################################################################
constexpr int MAXN = 128, MAXL = 6;
constexpr int MAX_BRANCH = 16;             // ★ 候補を絞ったので 5N=300 -> 16 で足りる
constexpr int BEAM_CAP   = 65536;          // ★ MAX_BRANCH を減らした分ビーム幅を大きく取れる

int N, MAX_TURN;
static int INIT_P[MAXN];
static uint64_t ZOB[MAXN][MAXN];

// ★ Move に「その手のスコア差分」を持たせておく。enum_moves で 1 度だけ計算すれば
//    calc_score は O(1) になり、O(L^2) の再計算が全部消える。
struct Move { int8_t i, len, d; };

struct State {
    float    score;         // 転倒数 + 支払ったコスト
    uint64_t hash;
    int8_t   p[MAXN];       // ★ int16_t -> int8_t (State のコピーが半分になる)
    int32_t  turn;
};

// MINB[L]: 長さ L までの区間内転倒数がこれ未満なら、それより長くしても差分が th 以下に
//          ならないことが確定する下限。ここで打ち切ると比較回数が約 1/4 減る。
static int MINB[MAXL + 1];
static void build_minb() {
    int need[MAXL + 2];
    for (int L = 2; L <= MAXL; L++) { const int T = L * (L - 1) / 2; need[L] = (T + L - MOVE_TH + 1) >> 1; }
    for (int L = 2; L <= MAXL; L++) {
        int m = INT_MAX, g = 0;
        for (int L2 = L + 1; L2 <= MAXL; L2++) { g += (L2 - 1); m = min(m, need[L2] - g); }
        MINB[L] = (m == INT_MAX) ? INT_MIN : m;
    }
}

template <class T> static inline int inversions(const T *p) {   // replay 側 (int16_t) と共用
    int t = 0;
    for (int i = 0; i < N; i++) for (int j = i + 1; j < N; j++) if (p[i] > p[j]) t++;
    return t;
}

void init_state(State &s) {
    build_minb();
    s.turn = 0; s.hash = 0;
    for (int i = 0; i < N; i++) { s.p[i] = (int8_t)INIT_P[i]; s.hash ^= ZOB[i][s.p[i]]; }
    s.score = (float)inversions(s.p);
}

// ---- 候補の絞り込み ---------------------------------------------------------
//  区間 [i,i+L) を反転したときのスコア差分は  L(L-1)/2 - 2*before + L  (before = 区間内の転倒数)。
//  L=2 なら最小 +1、L=3 なら最小 0 なので、得をする手は L>=4 の「かなり降順な区間」だけ。
//  そこで差分が MOVE_TH 以下の手だけを候補にする。1 手あたりの候補が 300 -> 30 程度になり、
//  同じ時間で実質のビーム幅が一桁大きくなる。候補が 1 つも無ければ終端 (そこで打ち切る) 扱い。
static Move g_tmp[MAXN * (MAXL - 1)];

inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= MAX_TURN) return 0;
    const int8_t *p = s.p;
    const int th = MOVE_TH;
    int m = 0;
    for (int i = 0; i + 2 <= N; i++) {
        const int Lmax = min(MAXL, N - i);
        int before = 0;
        for (int L = 2; L <= Lmax; L++) {
            const int v = p[i + L - 1];                 // before(i,L) を漸化で求める
            for (int a = 0; a < L - 1; a++) before += (p[i + a] > v);
            const int d = L * (L - 1) / 2 - 2 * before + L;
            if (d <= th) { g_tmp[m].i = (int8_t)i; g_tmp[m].len = (int8_t)L; g_tmp[m].d = (int8_t)d; m++; }
            if (before < MINB[L]) break;            // これ以上長くしても差分が th 以下にならない
        }
    }
    const int cap = min(MAX_BRANCH, TOPK);
    if (m <= cap) { for (int k = 0; k < m; k++) out[k] = g_tmp[k]; return m; }
    // 溢れたら差分の小さい順に cap 個だけ残す (カウンティングソート)
    int cnt[64] = {0};
    for (int k = 0; k < m; k++) cnt[g_tmp[k].d + 16]++;
    int acc = 0, lim = 63;
    for (int b = 0; b < 64; b++) { acc += cnt[b]; if (acc >= cap) { lim = b; break; } }
    int o = 0;
    for (int k = 0; k < m; k++) if (g_tmp[k].d + 16 <  lim) out[o++] = g_tmp[k];
    for (int k = 0; k < m && o < cap; k++) if (g_tmp[k].d + 16 == lim) out[o++] = g_tmp[k];
    return o;
}

inline float calc_score(const State &s, const Move &mv) { return (float)mv.d; }   // ★ O(1)

inline uint64_t calc_hash(const State &s, const Move &mv) {
    uint64_t h = s.hash;
    const int i = mv.i, L = mv.len;
    for (int a = 0; a < L; a++) {
        h ^= ZOB[i + a][s.p[i + a]];
        h ^= ZOB[i + a][s.p[i + L - 1 - a]];
    }
    return h;                       // 重複除去はターンごとなので turn を混ぜる必要はない
}
inline void apply_move(State &s, const Move &mv) {
    const int i = mv.i, L = mv.len;
    for (int a = 0; a < L; a++) s.hash ^= ZOB[i + a][s.p[i + a]];
    reverse(s.p + i, s.p + i + L);
    for (int a = 0; a < L; a++) s.hash ^= ZOB[i + a][s.p[i + a]];
    s.turn++;
}

void read_input() {
    if (scanf("%d %d", &N, &MAX_TURN) == 2 && N >= 2) {
        N = min(N, MAXN);
        for (int i = 0; i < N; i++) scanf("%d", &INIT_P[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 305);
        N = 60; MAX_TURN = 150;
        for (int i = 0; i < N; i++) INIT_P[i] = i;
        for (int i = N - 1; i > 0; i--) swap(INIT_P[i], INIT_P[g.next(i + 1)]);
    }
    Xor128 z; z.seed(24680);
    for (int i = 0; i < MAXN; i++) for (int v = 0; v < MAXN; v++) ZOB[i][v] = ((uint64_t)z.next() << 32) | z.next();
}

void output(const vector<Move> &path) {
    string r; r.reserve(path.size() * 8);
    for (const Move &mv : path) { r += to_string((int)mv.i); r += ' '; r += to_string((int)mv.i + (int)mv.len - 1); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &path) {
    static int16_t p[MAXN];
    for (int i = 0; i < N; i++) p[i] = (int16_t)INIT_P[i];
    float cost = 0.0f;
    if ((int)path.size() > MAX_TURN) { fprintf(stderr, "[error] 操作回数超過\n"); return -1.0f; }
    for (const Move &mv : path) {
        if (mv.i < 0 || mv.i + mv.len > N || mv.len < 2) { fprintf(stderr, "[error] 範囲外の操作\n"); return -1.0f; }
        reverse(p + mv.i, p + mv.i + mv.len);
        cost += (float)mv.len;
    }
    return (float)inversions(p) + cost;
}
// #############################################################################
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
