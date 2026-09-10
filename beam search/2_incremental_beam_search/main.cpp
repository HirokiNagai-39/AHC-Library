// =============================================================================
//  差分更新ビームサーチ (Euler tour Beam Search)   ---  AHC 用 高速テンプレート
// =============================================================================
//  State のコピーを一切しない。木を DFS で辿り apply/undo で状態を作り直す。State が大きいほど速い。
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
//    undo_move (State&, Move)           ... apply_move を巻き戻す (★差分更新版で必須)
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
// ★ビーム幅は最重要パラメータ。ADAPTIVE_WIDTH=true のときは
//   「基準値」として使われ、残り時間に応じて [BEAM_WIDTH/4, BEAM_WIDTH*2] の範囲で自動調整される。
//   実行後に [hint] が出たら大きくする。時間切れになるなら小さくする。
int   BEAM_WIDTH = 20000;  // p1: ビーム幅 (デモに合わせた値。新しい問題では 1000 くらいから始める)
float FUTURE_W   = 0.5f;   // p2: 評価関数の先読み項の重み (デモ用。問題ごとの評価関数の重みに相当)

void load_params() {
    pick_env("p1", BEAM_WIDTH);
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
constexpr int  MIN_BEAM_WIDTH = 1;      // ビーム幅の下限
constexpr bool ADAPTIVE_WIDTH = true;   // 残り時間からビーム幅を自動調整する (時間を使い切る)
constexpr bool USE_HASH_DEDUP = true;   // ハッシュによる重複除去を行う

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_expanded = 0, g_cand_total = 0;
static int g_turn_done = 0;
static float g_best_eval = 0.0f;

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

// ---- 差分更新用の巻き戻しスタック ----
//  apply / undo は必ず入れ子(DFS)で呼ばれるので、単純なスタックで元に戻せる。
static int32_t g_undo_pos[MAXT + 8];      // 移動前の位置
static bool    g_undo_mark[MAXT + 8];     // この手で新しくマークしたか
static int     g_undo_sp = 0;

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
    g_undo_pos[g_undo_sp] = s.pos;            // 巻き戻し情報を積む
    g_undo_mark[g_undo_sp] = fresh;
    g_undo_sp++;
    s.hash ^= Z_POS[s.pos] ^ Z_POS[c];
    if (fresh) { s.vis[wI] |= bit; s.hash ^= Z_VIS[c]; }
    s.pos = c;
}

// ★差分更新版では必須: apply_move を巻き戻す (状態を完全に元へ戻すこと)
inline void undo_move(State &s, const Move &mv) {
    (void)mv;
    --g_undo_sp;
    const int c = s.pos;
    if (g_undo_mark[g_undo_sp]) { s.vis[c >> 6] &= ~(1ULL << (c & 63)); s.hash ^= Z_VIS[c]; }
    const int p = g_undo_pos[g_undo_sp];
    s.hash ^= Z_POS[p] ^ Z_POS[c];
    s.pos = p;
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
    g_undo_sp = 0;
    State s; init_state(s);
    float total = (float)VAL[0];
    for (const Move &mv : path) {
        const int c = NB[s.pos][mv.dir];
        if (c < 0) { fprintf(stderr, "[error] 不正な手があります\n"); return -1.0f; }
        if (!((s.vis[c >> 6] >> (c & 63)) & 1)) total += (float)VAL[c];
        apply_move(s, mv);
    }
    g_undo_sp = 0;
    return total;
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
// 7. 差分更新ビームサーチ (Euler tour beam search) 本体
// =============================================================================
//  State のコピーを一切しない。持つのは「今のビームが作る木」だけで、状態は 1 個。
//  木を DFS(オイラーツアー)で辿り、潜るとき apply_move / 戻るとき undo_move して
//  葉に着いた瞬間だけ候補を列挙する。State が大きいほど通常版より速くなる。
//
//  tour[] は木のオイラーツアーそのもの:
//     id >= 0 … 葉 (apply → 候補列挙 → undo)
//     id == -1 … 潜る (apply)
//     id == -2 … 戻る (undo)
//  木は毎ターン作り直すが、子が 1 つも選ばれなかった枝はその場で刈られる。
struct Ev { Move mv; float score; int32_t id; };

static vector<Ev>      g_tour, g_ntour;           // 現在の木 / 次の木
static vector<int32_t> g_stk;                     // 枝刈り用スタック
static vector<Move>    g_path, g_term_path;       // ツアー走査中の現在の手順 / 終端解の手順
static float           g_term_best = WORST_SCORE;
static Cand    g_cand[BEAM_CAP * MAX_BRANCH];     // 候補
static Cand    g_sel[BEAM_CAP];                   // 親(葉)ごとに並べ替えた選択結果
static int32_t g_start[BEAM_CAP + 1], g_fill[BEAM_CAP + 1];

vector<Move> incremental_beam_search(float deadline_ms) {
    static State s;                               // ★状態はこの 1 個だけ
    init_state(s);

    g_tour.clear();  g_tour.reserve(1 << 18);
    g_ntour.reserve(1 << 18);
    g_stk.reserve(MAXT + 8);

    int   n_leaf  = 1;                            // 最初は根だけが葉
    int   width   = min(BEAM_WIDTH, BEAM_CAP);
    float prev_ms = timer.ms();
    Move  mvbuf[MAX_BRANCH];
    int   turn = 0;

    for (; turn < MAX_TURN; turn++) {
        // ---- 1) オイラーツアーを辿り、葉で候補を列挙する ----
        int nc = 0;
        if (g_tour.empty()) {                     // 最初のターンは根が唯一の葉
            const int m = enum_moves(s, mvbuf);
            for (int k = 0; k < m; k++) {
                Cand &c = g_cand[nc++];
                c.hash = calc_hash(s, mvbuf[k]); c.score = s.score + calc_score(s, mvbuf[k]);
                c.parent = 0; c.mv = mvbuf[k];
            }
        } else {
            for (const Ev &e : g_tour) {
                if (e.id == -2) { undo_move(s, e.mv); g_path.pop_back(); continue; }   // 戻る
                apply_move(s, e.mv);                                // 潜る
                if (e.id < 0) g_path.push_back(e.mv);
                if (e.id >= 0) {                                    // 葉
                    s.score = e.score;                              // 葉の評価値を入れ直す(誤差が溜まらない)
                    const int m = enum_moves(s, mvbuf);
                    if (m == 0 && is_better(s.score, g_term_best)) { // ★終端に到達: 手順を控えておく
                        g_term_best = s.score;
                        g_term_path.assign(g_path.begin(), g_path.end());
                        g_term_path.push_back(e.mv);
                    }
                    for (int k = 0; k < m; k++) {
                        Cand &c = g_cand[nc++];
                        c.hash = calc_hash(s, mvbuf[k]); c.score = s.score + calc_score(s, mvbuf[k]);
                        c.parent = e.id; c.mv = mvbuf[k];
                    }
                    undo_move(s, e.mv);                             // 葉はすぐ戻る
                }
            }
        }
        g_expanded += n_leaf; g_cand_total += nc;
        if (nc == 0) break;

        // ---- 2) 重複除去 → 3) 上位 width 個を選抜 ----
        if constexpr (USE_HASH_DEDUP) nc = dedup(g_cand, nc);
        nc = select_top(g_cand, nc, width);

        // ---- 4) 選ばれた候補を親(葉)ごとにまとめる (計数ソート O(n)) ----
        for (int i = 0; i <= n_leaf; i++) g_start[i] = 0;
        for (int i = 0; i < nc; i++) g_start[g_cand[i].parent + 1]++;
        for (int i = 0; i < n_leaf; i++) g_start[i + 1] += g_start[i];
        memcpy(g_fill, g_start, sizeof(int32_t) * (size_t)(n_leaf + 1));
        for (int i = 0; i < nc; i++) g_sel[g_fill[g_cand[i].parent]++] = g_cand[i];

        // ---- 5) 次のオイラーツアーを組み立てる (死んだ枝は刈る) ----
        g_ntour.clear(); g_stk.clear();
        int new_leaf = 0;
        if (g_tour.empty()) {
            for (int k = 0; k < nc; k++) g_ntour.push_back({g_sel[k].mv, g_sel[k].score, new_leaf++});
        } else {
            for (const Ev &e : g_tour) {
                if (e.id == -1) {                                   // 潜る: とりあえず積む
                    g_stk.push_back((int32_t)g_ntour.size());
                    g_ntour.push_back(e);
                } else if (e.id == -2) {                            // 戻る
                    if ((int32_t)g_ntour.size() == g_stk.back() + 1) g_ntour.pop_back();  // 中身が空 → 枝刈り
                    else g_ntour.push_back(e);
                    g_stk.pop_back();
                } else {                                            // 葉: 選ばれた子を新しい葉にする
                    const int b = g_start[e.id], en = g_start[e.id + 1];
                    if (b == en) continue;                          // 子が 1 つも選ばれなかった → 捨てる
                    g_ntour.push_back({e.mv, 0.0f, -1});
                    for (int k = b; k < en; k++)
                        g_ntour.push_back({g_sel[k].mv, g_sel[k].score, new_leaf++});
                    g_ntour.push_back({e.mv, 0.0f, -2});
                }
            }
        }
        swap(g_tour, g_ntour);
        const int processed = n_leaf;
        n_leaf = new_leaf;

        // ---- 6) 時間計測とビーム幅の調整 (1 ターンに 1 回だけ) ----
        const float now = timer.ms();
        if constexpr (ADAPTIVE_WIDTH) {
            const int rem = MAX_TURN - turn - 1;
            if (rem > 0) {
                const float per    = (now - prev_ms) / (float)max(1, processed);
                const float budget = (deadline_ms - now) / (float)rem;
                int nw = (int)(budget / max(per, 1e-9f));
                nw = min(nw, width * 2); nw = max(nw, width / 2);   // 1 ターンでの急変を抑える
                width = min(nw, min(BEAM_WIDTH * 2, BEAM_CAP));     // BEAM_WIDTH を基準に [1/4, 2] 倍
                width = max(width, max(MIN_BEAM_WIDTH, BEAM_WIDTH / 4));
            }
        }
        if (now >= deadline_ms) width = 1;
        prev_ms = now;
    }
    g_turn_done = turn;

    // ---- 最良の葉を探し、ツアーを辿って手順を復元する ----
    int32_t best_leaf = -1;
    float   best_sc   = WORST_SCORE;
    for (const Ev &e : g_tour) if (e.id >= 0 && is_better(e.score, best_sc)) { best_sc = e.score; best_leaf = e.id; }
    g_best_eval = best_sc;

    vector<Move> path, stk;
    stk.reserve(MAX_TURN + 1);
    for (const Ev &e : g_tour) {
        if (e.id == -2) { stk.pop_back(); continue; }
        stk.push_back(e.mv);
        if (e.id == best_leaf) { path = stk; break; }
        if (e.id >= 0) stk.pop_back();
    }
    // 途中で終端に達した解の方が良ければそちらを採用する
    if (path.empty() || (!g_term_path.empty() && is_better(g_term_best, best_sc))) {
        g_best_eval = g_term_best;
        return g_term_path;
    }
    return path;
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

    vector<Move> path = incremental_beam_search(TIME_LIMIT_MS);
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
