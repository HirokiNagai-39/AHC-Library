// =============================================================================
//  [ビームサーチ系 01] ブロック消し (SameGame)
// =============================================================================
//  【問題】
//    H×W のグリッドの各マスに C 色のいずれかのブロックがある。
//    1 手で「同じ色の 4 連結成分でサイズ k >= 2 のもの」を 1 つ選んで消せる。得点は (k-2)^2。
//    消した後、上のブロックが落下し、空になった列があれば右の列を左に詰める。
//    消せる成分が無くなるか T 手に達したら終了。総得点を最大化せよ。
//  【入力】
//    H W C T
//    色を H 行 W 列 (0..C-1)。上の行が上。
//  【出力】
//    消す成分の代表マスを 1 手ずつ "r c" の形で 1 行に 1 つ (手数ぶん)。
//  【スコア】 総得点 (大きいほど良い)。消せない成分を指定したら 0 点。
//  【入力生成方法】
//    H=W=12, C=4, T=80 固定。各マスの色は 0..C-1 の一様乱数。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     1 手ごとに盤面が大きく変わる (消す → 落下 → 空き列を詰める) ので、
//     「解を少し変える」局所探索が組みにくい。
//     一方「毎ターン候補を全部出して上位だけ残す」ビームサーチはそのまま適用できる。
//     この問題の要点は 2 つ。
//       ・1 手のシミュレーションを軽くする (展開数がそのまま強さになる)
//       ・「大きい塊を育てる」という先読みを評価値に入れる (得点が (k-2)^2 と非線形なので、
//         小さい成分を消すより大きく育ててから消す方が得)
//
//   ● 状態 (State)
//     盤面を「列ごとに下から積んだ色」で持つ。
//     この表現なら落下も空き列詰めも配列を詰めるだけで表せるので、
//     1 手のシミュレーションが盤面 1 走査で済み、そのついでにハッシュと評価項も作れる。
//
//   ● 手の作り方
//     「サイズ 2 以上の同色連結成分」が候補。
//     flood fill を全マスから繰り返すのではなく、1 回のラベリングで全成分をまとめて列挙する。
//     候補は成分の代表マス 1 つで表す。
//
//   ● 評価値
//     得点 (k-2)^2 + EVAL_W × (同色の隣接ペア数)
//     第 2 項が「大きい塊を残す」ための先読み項。
//     ここで大事なのは、この項が**ゲーム終了時 (サイズ 2 以上の成分が無い) にちょうど 0 になる**こと。
//     終端で 0 になる量を足し引きしても最終的な順位付けは変わらないので (ポテンシャル型の報酬整形)、
//     EVAL_W をいくら大きくしても「真のスコアが最大の解」を見失わない。
//     実際 0.5 -> 22 まで上げて +49% になった。
//
//   ● 差分計算 / 高速化
//     消去後の盤面を作る走査の中で、得点・隣接ペア数・Zobrist ハッシュを同時に計算する。
//     以前は calc_score / calc_hash / apply_move でそれぞれシミュレーションしていて 3 倍無駄だった。
//     これを 1 回に統合して展開数が約 6 倍になった。
//
//   ● つまずきポイント
//     ・連結成分の個数はサイズ 1 も含めると盤面のマス数近くまで増える。
//       候補配列を MAX_BRANCH ぴったりで確保すると溢れる ([verify] が検出してくれる)。
//     ・chokudai サーチは状態プールの上限で頭打ちになる。高速化して時間が余るようになった段階で、
//       通常のビームサーチに移して幅を広げた方が伸びた。
//
//   ● さらに伸ばすなら
//     ・先読み項を「同色ペア数」から「連結成分のサイズの二乗和」にする (得点の形に近い)
//     ・色ごとの残数を見て「最後に大きく消す色」を決め打ちする方向
//
//  【改善】
//    (1) 状態をグリッドから列表現に変え、calc_score / calc_hash / apply_move で 3 回走っていた
//        シミュレーションを 1 回に統合。成分列挙も flood fill の繰り返しから 1 回のラベリングに変更。
//        展開数が約 6 倍になった。
//    (2) EVAL_W を 0.5 -> 22 に。先読み項が終端で 0 になるポテンシャル型なので、大きくしても
//        最適解の順位は変わらず、途中の誘導だけが強くなる (ここが一番効いた)。
//    (3) 高速化で時間が余るようになったため chokudai サーチ (プール上限で頭打ち) から
//        通常のビームサーチに移植し直し、BEAM_WIDTH 16000 / BEAM_CAP 32768 に拡大。
//    seed 0..4 合計: 変更前 5197 -> 変更後 7770 (+49.5%)
//
//  【採用したライブラリ】 beam search/1_beam_search
//    実測比較 (seed 0..4 の合計スコア, 高速化後の評価関数で): 1_beam_search=7656 / 3_chokudai_search=7488
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
int   BEAM_WIDTH = 16000;      // p1: ビーム幅 (ADAPTIVE_WIDTH=true なら基準値)
float EVAL_W     = 22.0f;      // p2: 同色の隣接対を残すことへの重み
float EVAL_W2    = 0.0f;       // p3: Σ(成分サイズ-2)^2 (今すぐ取れる得点) への重み

void load_params() {
    pick_env("p1", BEAM_WIDTH);
    pick_env("p2", EVAL_W);
    pick_env("p3", EVAL_W2);
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
constexpr int  MIN_BEAM_WIDTH = 1;      // ビーム幅の下限
constexpr bool ADAPTIVE_WIDTH = true;   // 残り時間からビーム幅を自動調整する (時間を使い切る)
constexpr bool USE_HASH_DEDUP = true;   // ハッシュによる重複除去を行う

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_expanded = 0, g_cand_total = 0;
static int g_turn_done = 0;
static float g_best_eval = 0.0f;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (ブロック消し / SameGame)
// #############################################################################
constexpr int MAXH = 16, MAXW = 16, MAXC = MAXH * MAXW;
constexpr int MAXT = 200;
constexpr int MAX_BRANCH = 48;              // ★1 手の候補数の上限 (12x12/4色なら実測 35 以下)
constexpr int BEAM_CAP   = 32768;            // ★ビーム幅の上限

int H, W, NCOL, MAX_TURN;
static uint64_t ZOB[MAXW][MAXH][8];          // (列, 下からの段, 色)

struct Move { int8_t r, c; int16_t id; };    // 出力用の代表マス + 成分 id

// ---- 状態は「列ごとに下から積んだ色」で持つ ----
//   落下と列詰めがこの表現なら自然に表せるので、1 手のシミュレーションが盤面 1 走査で済む。
struct State {
    float    score;
    uint64_t hash;
    int8_t   col[MAXW][MAXH];   // 下から順の色
    int8_t   ht[MAXW];          // 各列の高さ
    int8_t   nw;                // 空でない列の本数
    int32_t  turn;
    float    ev1;               // 同色隣接ペア数 (キャッシュ)
    float    ev2;               // Σ(成分サイズ-2)^2 (キャッシュ)
};

static int8_t G0[MAXC];
static inline int at(int r, int c) { return r * W + c; }

// =========================== 成分ラベリング ===========================
static int16_t g_lab[MAXW][MAXH];
static int     g_csz[MAXC + 4];
static int     g_ncomp;
static uint64_t g_lab_key_h = ~0ULL;
static int      g_lab_key_t = -1;

// 列表現の盤面に対して連結成分をラベル付けし、成分数を返す
static inline int label(const int8_t (*col)[MAXH], const int8_t *ht, int nw,
                        int16_t (*lab)[MAXH], int *csz) {
    for (int c = 0; c < nw; c++) for (int y = 0; y < ht[c]; y++) lab[c][y] = -1;
    static int st[MAXC];
    int nc = 0;
    for (int c0 = 0; c0 < nw; c0++) for (int y0 = 0; y0 < ht[c0]; y0++) {
        if (lab[c0][y0] >= 0) continue;
        const int color = col[c0][y0];
        const int id = nc++;
        int sp = 0, cnt = 0;
        st[sp++] = c0 * MAXH + y0; lab[c0][y0] = (int16_t)id;
        while (sp) {
            const int v = st[--sp]; cnt++;
            const int c = v / MAXH, y = v % MAXH;
            if (y + 1 < ht[c]      && lab[c][y+1] < 0 && col[c][y+1] == color) { lab[c][y+1] = (int16_t)id; st[sp++] = c * MAXH + y + 1; }
            if (y - 1 >= 0         && lab[c][y-1] < 0 && col[c][y-1] == color) { lab[c][y-1] = (int16_t)id; st[sp++] = c * MAXH + y - 1; }
            if (c + 1 < nw && y < ht[c+1] && lab[c+1][y] < 0 && col[c+1][y] == color) { lab[c+1][y] = (int16_t)id; st[sp++] = (c+1) * MAXH + y; }
            if (c - 1 >= 0 && y < ht[c-1] && lab[c-1][y] < 0 && col[c-1][y] == color) { lab[c-1][y] = (int16_t)id; st[sp++] = (c-1) * MAXH + y; }
        }
        csz[id] = cnt;
    }
    return nc;
}

// =========================== 1 手の結果 (キャッシュ付き) ===========================
struct SimRes {
    int8_t   col[MAXW][MAXH];
    int8_t   ht[MAXW];
    int      nw;
    uint64_t hash;
    float    ev1, ev2;
    int      n;                 // 消した個数
};
static SimRes   g_sim;
static uint64_t g_sim_key_h = ~0ULL;
static int      g_sim_key_t = -1, g_sim_key_id = -1;

static int16_t g_lab2[MAXW][MAXH];
static int     g_csz2[MAXC + 4];

// s の成分ラベルを (無ければ) 作る。同じ状態を何度も触るのでキャッシュしておく。
static inline void ensure_lab(const State &s);

// 成分 id を消して落下・列詰めした盤面を g_sim に作る。ハッシュと評価項も同じ走査で作る。
static void do_sim(const State &s, int id) {
    ensure_lab(s);
    int dst = 0, removed = 0, clump = 0;
    uint64_t h = 0;
    for (int c = 0; c < s.nw; c++) {
        const int hc = s.ht[c];
        int8_t *out = g_sim.col[dst];
        int w = 0;
        for (int y = 0; y < hc; y++) {
            if (g_lab[c][y] == id) { removed++; continue; }
            out[w++] = s.col[c][y];
        }
        if (w == 0) continue;
        g_sim.ht[dst] = (int8_t)w;
        for (int y = 0; y < w; y++) {
            h ^= ZOB[dst][y][out[y]];
            if (y + 1 < w && out[y + 1] == out[y]) clump++;
        }
        if (dst > 0) {
            const int8_t *pl = g_sim.col[dst - 1];
            const int mn = min((int)g_sim.ht[dst - 1], w);
            for (int y = 0; y < mn; y++) if (pl[y] == out[y]) clump++;
        }
        dst++;
    }
    g_sim.nw = dst; g_sim.hash = h; g_sim.n = removed; g_sim.ev1 = (float)clump;
    if (EVAL_W2 != 0.0f) {                       // Σ(成分サイズ-2)^2 (「今すぐ取れる得点」の指標)
        const int nc = label(g_sim.col, g_sim.ht, dst, g_lab2, g_csz2);
        float p = 0.0f;
        for (int i = 0; i < nc; i++) if (g_csz2[i] >= 2) { const float d = (float)(g_csz2[i] - 2); p += d * d; }
        g_sim.ev2 = p;
    } else g_sim.ev2 = 0.0f;
}
static inline void ensure_lab(const State &s) {
    if (g_lab_key_h == s.hash && g_lab_key_t == s.turn) return;
    g_ncomp = label(s.col, s.ht, s.nw, g_lab, g_csz);
    g_lab_key_h = s.hash; g_lab_key_t = s.turn;
}
static inline void ensure_sim(const State &s, int id) {
    if (g_sim_key_h == s.hash && g_sim_key_t == s.turn && g_sim_key_id == id) return;
    do_sim(s, id);
    g_sim_key_h = s.hash; g_sim_key_t = s.turn; g_sim_key_id = id;
}

// =========================== フレームワークとの接続 ===========================
static inline void refresh_ev(State &s) {
    const int nc = label(s.col, s.ht, s.nw, g_lab2, g_csz2);
    float p = 0.0f;
    for (int i = 0; i < nc; i++) if (g_csz2[i] >= 2) { const float d = (float)(g_csz2[i] - 2); p += d * d; }
    s.ev2 = p;
    int clump = 0;
    for (int c = 0; c < s.nw; c++) {
        for (int y = 0; y + 1 < s.ht[c]; y++) if (s.col[c][y] == s.col[c][y + 1]) clump++;
        if (c > 0) { const int mn = min((int)s.ht[c - 1], (int)s.ht[c]); for (int y = 0; y < mn; y++) if (s.col[c - 1][y] == s.col[c][y]) clump++; }
    }
    s.ev1 = (float)clump;
}

void init_state(State &s) {
    s.score = 0.0f; s.turn = 0;
    s.nw = (int8_t)W;
    uint64_t h = 0;
    for (int c = 0; c < W; c++) {
        s.ht[c] = (int8_t)H;
        for (int y = 0; y < H; y++) { s.col[c][y] = G0[at(H - 1 - y, c)]; h ^= ZOB[c][y][s.col[c][y]]; }
    }
    s.hash = h;
    refresh_ev(s);
}

inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= MAX_TURN) return 0;
    ensure_lab(s);
    static uint8_t used[MAXC + 4];
    for (int i = 0; i < g_ncomp; i++) used[i] = 0;
    int m = 0;
    for (int c = 0; c < s.nw && m < MAX_BRANCH; c++)
        for (int y = 0; y < s.ht[c] && m < MAX_BRANCH; y++) {
            const int id = g_lab[c][y];
            if (g_csz[id] < 2 || used[id]) continue;
            used[id] = 1;
            out[m].r = (int8_t)(H - 1 - y); out[m].c = (int8_t)c; out[m].id = (int16_t)id; m++;
        }
    return m;
}

inline float calc_score(const State &s, const Move &mv) {
    ensure_sim(s, mv.id);
    const float d = (float)(g_sim.n - 2);
    return d * d + EVAL_W * (g_sim.ev1 - s.ev1) + EVAL_W2 * (g_sim.ev2 - s.ev2);
}
inline uint64_t calc_hash(const State &s, const Move &mv) {
    ensure_sim(s, mv.id);
    return g_sim.hash ^ (uint64_t)(s.turn + 1) * 0x9E3779B97F4A7C15ULL;
}
inline void apply_move(State &s, const Move &mv) {
    ensure_sim(s, mv.id);
    memcpy(s.col, g_sim.col, sizeof(int8_t) * (size_t)MAXW * MAXH);
    memcpy(s.ht, g_sim.ht, sizeof(int8_t) * (size_t)MAXW);
    s.nw = (int8_t)g_sim.nw;
    s.ev1 = g_sim.ev1; s.ev2 = g_sim.ev2;
    s.turn++;
    s.hash = g_sim.hash ^ (uint64_t)s.turn * 0x9E3779B97F4A7C15ULL;
}

// =========================== 入出力 / 採点 (グリッド表現) ===========================
// (r,c) を含む同色連結成分を集めてサイズを返す (out に位置を入れる)
static int flood(const int8_t *g, int r, int c, int *out) {
    static int st[MAXC]; static uint8_t vis[MAXC];
    const int col = g[at(r, c)];
    if (col < 0) return 0;
    memset(vis, 0, sizeof(uint8_t) * (size_t)(H * W));
    int sp = 0, n = 0;
    st[sp++] = at(r, c); vis[at(r, c)] = 1;
    while (sp) {
        const int v = st[--sp];
        out[n++] = v;
        const int vr = v / W, vc = v % W;
        const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
        for (int k = 0; k < 4; k++) {
            const int nr = vr + dr[k], nc = vc + dc[k];
            if (nr < 0 || nr >= H || nc < 0 || nc >= W) continue;
            const int u = at(nr, nc);
            if (!vis[u] && g[u] == col) { vis[u] = 1; st[sp++] = u; }
        }
    }
    return n;
}

// 成分を消して落下・列詰めまで行った盤面を dst に作る
static int simulate(const int8_t *g, int r, int c, int8_t *dst) {
    static int cells[MAXC];
    const int n = flood(g, r, c, cells);
    memcpy(dst, g, sizeof(int8_t) * (size_t)(H * W));
    if (n < 2) return 0;
    for (int i = 0; i < n; i++) dst[cells[i]] = -1;
    // 落下
    for (int j = 0; j < W; j++) {
        int w = H - 1;
        for (int i = H - 1; i >= 0; i--) if (dst[at(i, j)] >= 0) dst[at(w--, j)] = dst[at(i, j)];
        while (w >= 0) dst[at(w--, j)] = -1;
    }
    // 空の列を左に詰める
    int wc = 0;
    for (int j = 0; j < W; j++) {
        bool empty = true;
        for (int i = 0; i < H; i++) if (dst[at(i, j)] >= 0) { empty = false; break; }
        if (empty) continue;
        if (wc != j) for (int i = 0; i < H; i++) { dst[at(i, wc)] = dst[at(i, j)]; dst[at(i, j)] = -1; }
        wc++;
    }
    return n;
}

void read_input() {
    if (scanf("%d %d %d %d", &H, &W, &NCOL, &MAX_TURN) == 4 && H >= 1) {
        H = min(H, MAXH); W = min(W, MAXW); MAX_TURN = min(MAX_TURN, MAXT);
        for (int i = 0; i < H * W; i++) { int v; scanf("%d", &v); G0[i] = (int8_t)v; }
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 301);
        H = 12; W = 12; NCOL = 4; MAX_TURN = 80;
        for (int i = 0; i < H * W; i++) G0[i] = (int8_t)g.next((uint32_t)NCOL);
    }
    Xor128 z; z.seed(777);
    for (int c = 0; c < MAXW; c++) for (int y = 0; y < MAXH; y++) for (int k = 0; k < 8; k++) ZOB[c][y][k] = ((uint64_t)z.next() << 32) | z.next();
}

void output(const vector<Move> &path) {
    string r; r.reserve(path.size() * 6);
    for (const Move &mv : path) { r += to_string((int)mv.r); r += ' '; r += to_string((int)mv.c); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &path) {
    static int8_t g[MAXC], nx[MAXC];
    memcpy(g, G0, sizeof(int8_t) * (size_t)(H * W));
    float total = 0.0f;
    for (const Move &mv : path) {
        const int n = simulate(g, mv.r, mv.c, nx);
        if (n < 2) { fprintf(stderr, "[error] 消せない成分を指定\n"); return -1.0f; }
        total += (float)(n - 2) * (float)(n - 2);
        memcpy(g, nx, sizeof(int8_t) * (size_t)(H * W));
    }
    return total;
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
