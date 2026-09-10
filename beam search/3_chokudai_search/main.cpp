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
int   CHOKUDAI_WIDTH = 1;    // p1: 1 周で各深さから何個ずつ展開するか (普通は 1〜4)
float FUTURE_W       = 0.5f; // p2: 評価関数の先読み項の重み (デモ用)

void load_params() {
    pick_env("p1", CHOKUDAI_WIDTH);
    pick_env("p2", FUTURE_W);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化のとき こちらを有効化 (デモは最大化)
// constexpr bool MAXIMIZE = false;    // ← スコア最小化のとき こちらを有効化

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
// # 6. ■ ここから 問題ごとに書き換える部分 ■
// #     (デモ: グリッドお宝集め / 集めた価値の合計を最大化)
// #       H×W グリッドの各マスに価値がある。(0,0) から出発し毎ターン上下左右に 1 マス動く。
// #       初めて訪れたマスの価値をもらえる。MAX_TURN ターン後の合計を最大化する。
// #       標準入力が無ければランダムに問題を生成するので、そのまま実行できる。
// #############################################################################
constexpr int MAXH = 32, MAXW = 32;        // グリッドの上限
constexpr int MAXC = MAXH * MAXW;          // マス数の上限
constexpr int VIS_W = (MAXC + 63) / 64;    // 訪問フラグのワード数
constexpr int MAXT  = 2000;                // ターン数の上限
constexpr int MAX_BRANCH = 4;              // ★1 状態から出る遷移の最大数 (配列サイズに効く)
// ★ビーム幅の上限。State のサイズ×2×BEAM_CAP と 24×BEAM_CAP×MAX_BRANCH の静的メモリを使うので、
//   BEAM_CAP × MAX_BRANCH が 100 万を大きく超えないように問題ごとに調整すること。
constexpr int BEAM_CAP = 32768;

int H, W, MAX_TURN;                        // 入力
static int      VAL[MAXC];                 // 各マスの価値
static int32_t  NB[MAXC][4];               // 隣接マス (-1 なら壁)
static uint64_t Z_POS[MAXC], Z_VIS[MAXC];  // Zobrist ハッシュ用の乱数

// ★必須: 遷移(手)。小さいほど候補配列が cache に載るので速い。
struct Move { int8_t dir; };               // 0:上 1:下 2:左 3:右

// ★必須: 状態。float score を必ず持たせる。
struct State {
    float    score;        // ★必須: 評価値 (探索の順位付けに使う)
    uint64_t hash;         // ★必須(重複除去を使う場合)
    uint64_t vis[VIS_W];   // ここから下は問題ごとに自由
    int32_t  pos;
};

// ★必須: 初期状態
void init_state(State &s) {
    memset(s.vis, 0, sizeof(s.vis));
    s.pos   = 0;
    s.vis[0] = 1;                          // (0,0) は訪問済み
    s.hash  = Z_POS[0] ^ Z_VIS[0];
    s.score = (float)VAL[0];
}

// ★必須: 遷移候補を全部 out[] に詰めて個数を返す (0 を返したらその状態は終端)
inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    const int32_t *nb = NB[s.pos];
    for (int d = 0; d < 4; d++) if (nb[d] >= 0) out[m++].dir = (int8_t)d;
    return m;
}

// 未訪問の隣接マス数 (デモの評価関数で使う)
inline int free_nb(const State &s, int c) {
    int k = 0;
    for (int d = 0; d < 4; d++) {
        const int n = NB[c][d];
        if (n >= 0 && !((s.vis[n >> 6] >> (n & 63)) & 1)) k++;
    }
    return k;
}

// ★必須: その手を指したときの「スコア(評価値)差分」
//   デモの評価値 = 集めた価値の合計 + FUTURE_W * (現在地に隣接する未訪問マス数)
//   第2項は「行き止まりに入らない」ための先読み項。FUTURE_W=0 なら評価値=真のスコア。
inline float calc_score(const State &s, const Move &mv) {
    const int c = NB[s.pos][mv.dir];
    const bool fresh = !((s.vis[c >> 6] >> (c & 63)) & 1);
    const float gain = fresh ? (float)VAL[c] : 0.0f;
    return gain + FUTURE_W * (float)(free_nb(s, c) - free_nb(s, s.pos));
}

// ★重複除去を使うなら必須: その手を指したときのハッシュ (Zobrist の差分更新)
inline uint64_t calc_hash(const State &s, const Move &mv) {
    const int c = NB[s.pos][mv.dir];
    uint64_t h = s.hash ^ Z_POS[s.pos] ^ Z_POS[c];
    if (!((s.vis[c >> 6] >> (c & 63)) & 1)) h ^= Z_VIS[c];
    return h;
}

// ★必須: 手を実際に適用する (score は触らない。hash は自分で更新する)
inline void apply_move(State &s, const Move &mv) {
    const int c = NB[s.pos][mv.dir];
    const int wI = c >> 6;
    const uint64_t bit = 1ULL << (c & 63);
    const bool fresh = !(s.vis[wI] & bit);
    s.hash ^= Z_POS[s.pos] ^ Z_POS[c];
    if (fresh) { s.vis[wI] |= bit; s.hash ^= Z_VIS[c]; }
    s.pos = c;
}

// 入力 (デモ: 標準入力が無ければランダム生成)
void read_input() {
    if (scanf("%d %d %d", &H, &W, &MAX_TURN) == 3 && H >= 2 && W >= 2) {
        H = min(H, MAXH); W = min(W, MAXW); MAX_TURN = min(MAX_TURN, MAXT);
        for (int i = 0; i < H * W; i++) scanf("%d", &VAL[i]);
    } else {
        H = 30; W = 30; MAX_TURN = 600;
        Xor128 g; g.seed(20260824);
        for (int i = 0; i < H * W; i++) VAL[i] = (int)g.next(10);
    }
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++) {
            const int c = i * W + j;
            NB[c][0] = (i > 0)     ? c - W : -1;
            NB[c][1] = (i < H - 1) ? c + W : -1;
            NB[c][2] = (j > 0)     ? c - 1 : -1;
            NB[c][3] = (j < W - 1) ? c + 1 : -1;
        }
    Xor128 g; g.seed(998244353);
    for (int i = 0; i < H * W; i++) { Z_POS[i] = g.next64(); Z_VIS[i] = g.next64(); }
}

// 出力 (手順を出力する)
void output(const vector<Move> &path) {
    static const char DC[4] = {'U', 'D', 'L', 'R'};
    string res;
    res.reserve(path.size());
    for (const Move &mv : path) res += DC[mv.dir];
    printf("%s\n", res.c_str());
}

// 手順を再生して「真のスコア」を計算する (差分計算のバグ検出用)
float replay_true_score(const vector<Move> &path) {
    State s; init_state(s);
    float total = (float)VAL[0];
    for (const Move &mv : path) {
        const int c = NB[s.pos][mv.dir];
        if (c < 0) { fprintf(stderr, "[error] 不正な手があります\n"); return -1.0f; }
        if (!((s.vis[c >> 6] >> (c & 63)) & 1)) total += (float)VAL[c];
        apply_move(s, mv);
    }
    return total;
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
