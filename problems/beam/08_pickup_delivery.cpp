// =============================================================================
//  [ビームサーチ系 08] グリッド配送ロボット (集荷と配達)
// =============================================================================
//  【問題】
//    H×W のグリッドにロボットが 1 台いる (最初は (0,0))。荷物が K 個あり、
//    荷物 k は セル s_k にあり、セル d_k へ運ぶ必要がある。ロボットは同時に 1 個しか運べない。
//    毎ターン、上下左右のいずれかに 1 マス動く。移動後、次が自動的に起こる。
//      ・何も持っていなくて、そのセルに未集荷の荷物があれば拾う (番号が小さい方)
//      ・荷物 k を持っていて、そのセルが d_k なら配達完了
//    T ターンで配達完了した荷物の数を最大化せよ。
//  【入力】
//    H W K T
//    si sj di dj   (K 行。集荷セルと配達セルの座標)
//  【出力】
//    T 行。各ターンの移動 "U"/"D"/"L"/"R"。
//  【スコア】 配達完了した荷物の数 (大きいほど良い)。範囲外への移動があれば 0 点。
//  【入力生成方法】
//    H=W=14, K=20, T=140 固定。集荷セル・配達セルは [0,H)×[0,W) の一様ランダム (重複可)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     1 手の選択肢が 4 通りの逐次決定なのでビームサーチが素直に効く。
//     難所は評価値。素点は「配達した瞬間に +1」しか入らないので、
//     そのままだと評価値がほとんど平坦で、探索が盤面をうろうろするだけになる。
//     そこで「今向かうべき目標セルまでの距離」を引いて勾配を作る。
//
//   ● 状態 (State)
//     (位置, 運んでいる荷物, 集荷済みビット, 配達済みビット, ターン)。
//     荷物 20 個なので整数 1 本で足りる。guide 値 (下記) もキャッシュしておく。
//
//   ● 手の作り方
//     上下左右の 4 方向 (盤外に出る向きは除く)。
//     移動後に「手ぶらで未集荷の荷物の上 → 拾う」「運んでいる荷物の配達先 → 配達完了」が自動で起きる。
//
//   ● 評価値
//     配達数 - EVAL_W × (今向かうべき目標セルまでのマンハッタン距離の変化)
//     ・手ぶらなら「一番近い未集荷の荷物」、運んでいるなら「その配達先」までの距離。
//     ・**差分で入れる**のがポイント (前のターンの距離との差)。こうすると経路の途中で足し引きが
//       telescoping して打ち消し合い、最終的な評価値は「配達数 - 定数」になる。
//     ・さらに終端 1 ターンで距離項の重みを 0 に落とす (EVAL_DEC)。
//       そうしないと「配達数は少ないが目標に近い状態」が最後に勝ってしまうことがある。
//     ・EVAL_W は 0.3 だと強すぎて 0.15 が最良だった。
//
//   ● 差分計算 / 高速化
//     集荷判定はセルごとの荷物ビットマスク PK_AT[] で O(1)。
//     guide 値は State にキャッシュして、親のぶんを再計算しない。
//     これで BEAM_WIDTH を 8000 -> 16384 に上げられた。
//
//   ● つまずきポイント
//     ・先読み項を差分でなく絶対値で入れると、終端の比較が壊れる。
//     ・「一番近い未集荷」を毎回全荷物ループで探すと重い。キャッシュか、荷物数を絞る工夫が要る。
//
//   ● さらに伸ばすなら
//     ・別に書いた厳密な巡回順 DP による上界が seed 0-4 で 68 に対し現状 65 なので、
//       伸びしろは 5% 程度
//     ・「次に取りに行く荷物」を高レベルの手として選び、そこまでの移動を最短路で埋める
//       (2 段階の定式化) と、探索空間が一気に狭くなる
//
//  【改善】
//    (1) step() の集荷判定を前計算ビットマスクで O(1) 化 + guide 値を State にキャッシュ。
//        これで BEAM_WIDTH を 8000 -> 16384 (上限 32768) に上げられた。
//    (2) 距離項の重み EVAL_W を 0.3 -> 0.15 に。0.3 だと「配達数は少ないが目標に近い状態」が
//        終端で勝ってしまうことがあった。さらに終端 1 ターンで距離項の重みを 0 に落とし
//        (EVAL_DEC)、評価値が telescoping して最終比較が純粋な配達数になるようにした。
//    seed 0..4 合計: 変更前 63 -> 変更後 65 (+3.2%)   ※ seed 0..9 では 131 -> 133 (+1.5%)
//    ※ 別に書いた厳密な巡回順 DP による上界は seed 0..4 で 68 なので、ほぼ上限まで来ている。
//
//  【採用したライブラリ】 beam search/1_beam_search
//    実測比較 (seed 0,1,2 の合計スコア): 1_beam_search=38 / 3_chokudai_search=37
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
int   BEAM_WIDTH = 16384;  // p1: ビーム幅 (ADAPTIVE_WIDTH=true なら基準値)
float EVAL_W     = 0.15f;  // p2: 目標セルへの距離を評価に使う重み
float EVAL_B     = 0.0f;   // p3: 手ぶらのとき「集荷地->配達地の距離」をどれだけ目標選びに考慮するか
float EVAL_R     = 0.0f;   // p4: 荷物を拾うごとのボーナス (1 ターンあたり累積)
float EVAL_C     = 0.0f;   // p5: 目標選びで「配達地から次の集荷地までの距離」を考慮する重み
float EVAL_DEC   = 1.0f;   // p6: 終盤 DEC ターンで距離項を 0 に落とす (終端での順位歪みを消す)

void load_params() {
    pick_env("p1", BEAM_WIDTH);
    pick_env("p2", EVAL_W);
    pick_env("p3", EVAL_B);
    pick_env("p4", EVAL_R);
    pick_env("p5", EVAL_C);
    pick_env("p6", EVAL_DEC);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (グリッド配送ロボット)
// #############################################################################
constexpr int MAXH = 20, MAXW = 20, MAXPK = 24, MAXTT = 400;
constexpr int MAX_BRANCH = 4;
constexpr int BEAM_CAP   = 65536;          // 幅 32768 で飽和するのでこれで十分

int H, W, NPK, MAX_TURN;
static int SR[MAXPK], SC[MAXPK], DR[MAXPK], DC[MAXPK];
// ---- 前計算 ----------------------------------------------------------------
static uint32_t PK_AT[MAXH * MAXW];        // そのセルに集荷地がある荷物のビット集合
static int      SD[MAXPK];                 // 集荷地 -> 配達地 のマンハッタン距離
static int      NXT[MAXPK];                // 配達地 -> 一番近い別の荷物の集荷地 までの距離
static uint32_t FULLPK;

struct Move { int8_t d; };

struct State {
    float    score;
    uint64_t hash;
    uint32_t picked, done;
    float    g;              // この状態の guide 値 (親の分を再計算しないためのキャッシュ)
    int16_t  r, c;
    int8_t   carry;          // -1 なら手ぶら
    int32_t  turn;
};

static const int DX[4] = {-1, 1, 0, 0}, DY[4] = {0, 0, -1, 1};

// 今向かうべきセルまでの距離
//   持っている  : 配達先までの距離
//   手ぶら      : 「そこまでの距離 + EVAL_B×(集荷->配達の距離)」を最小にする荷物を目標に選び、
//                 値としてはその荷物の集荷地までの距離を返す。
//   ★ 目標の選び方だけに配達距離を混ぜるのが肝。値そのものに混ぜると、安い荷物を配り終える
//      ほどポテンシャルが上がって「配るほど損」になり逆効果だった (実測 63 -> 60)。
static inline float guide(int r, int c, int carry, uint32_t picked) {
    if (carry >= 0) return (float)(abs(r - DR[carry]) + abs(c - DC[carry]));
    float bestv = 1e9f; int bestd = 0;
    for (uint32_t m = ~picked & FULLPK; m; m &= m - 1) {
        const int k = __builtin_ctz(m);
        const int   d = abs(r - SR[k]) + abs(c - SC[k]);
        const float v = (float)d + EVAL_B * (float)SD[k] + EVAL_C * (float)NXT[k];  // 選ぶ基準にだけ使う
        if (v < bestv) { bestv = v; bestd = d; }
    }
    return (bestv > 1e8f) ? 0.0f : (float)bestd;            // 値そのものは「集荷地までの距離」
}

void init_state(State &s) {
    s.score = 0.0f; s.turn = 0; s.r = 0; s.c = 0; s.carry = -1; s.picked = 0; s.done = 0; s.hash = 0;
    s.g = guide(0, 0, -1, 0);
}

inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= MAX_TURN) return 0;
    int m = 0;
    for (int k = 0; k < 4; k++) {
        const int nr = s.r + DX[k], nc = s.c + DY[k];
        if (nr >= 0 && nr < H && nc >= 0 && nc < W) out[m++].d = (int8_t)k;
    }
    return m;
}

// 移動後の自動処理まで含めた次状態 (集荷判定を前計算ビットマスクで O(1) 化)
static inline void step(const State &s, int d, int &r, int &c, int &carry, uint32_t &picked, uint32_t &done, int &deliv) {
    r = s.r + DX[d]; c = s.c + DY[d]; carry = s.carry; picked = s.picked; done = s.done; deliv = 0;
    if (carry >= 0) {
        if (DR[carry] == r && DC[carry] == c) { done |= 1u << carry; carry = -1; deliv = 1; }
    } else {
        const uint32_t av = PK_AT[r * MAXW + c] & ~picked;
        if (av) { const int k = __builtin_ctz(av); picked |= 1u << k; carry = k; }
    }
}

// 距離項の重み。終端 EVAL_DEC ターンで 0 に落とす。
//  こうすると評価値が telescoping して最終ターンでは「配達数」だけの比較になり、
//  「配達数は少ないが目標に近い状態」が勝ってしまう終端バイアスが消える。
static inline float dec_w(int t) {
    const float rem = (float)(MAX_TURN - t);
    return (rem >= EVAL_DEC) ? 1.0f : (rem / EVAL_DEC);
}

inline float calc_score(const State &s, const Move &mv) {
    int r, c, carry, deliv; uint32_t picked, done;
    step(s, mv.d, r, c, carry, picked, done, deliv);
    float v = (float)deliv
            - EVAL_W * (dec_w(s.turn + 1) * guide(r, c, carry, picked) - dec_w(s.turn) * s.g);
    if (EVAL_R != 0.0f) v -= EVAL_R * (float)__builtin_popcount(picked);
    return v;
}
inline uint64_t calc_hash(const State &s, const Move &mv) {
    int r, c, carry, deliv; uint32_t picked, done;
    step(s, mv.d, r, c, carry, picked, done, deliv);
    return ((uint64_t)(r * W + c) * 0x9E3779B97F4A7C15ULL) ^ ((uint64_t)(carry + 2) * 0xC2B2AE3D27D4EB4FULL)
         ^ ((uint64_t)picked * 0xD6E8FEB86659FD93ULL) ^ ((uint64_t)done * 0xA24BAED4963EE407ULL)
         ^ ((uint64_t)(s.turn + 1) * 0x9E3779B97F4A7C15ULL);
}
inline void apply_move(State &s, const Move &mv) {
    int r, c, carry, deliv; uint32_t picked, done;
    step(s, mv.d, r, c, carry, picked, done, deliv);
    s.r = (int16_t)r; s.c = (int16_t)c; s.carry = (int8_t)carry; s.picked = picked; s.done = done; s.turn++;
    s.g = guide(r, c, carry, picked);
    s.hash = ((uint64_t)(r * W + c) * 0x9E3779B97F4A7C15ULL) ^ ((uint64_t)(carry + 2) * 0xC2B2AE3D27D4EB4FULL)
           ^ ((uint64_t)picked * 0xD6E8FEB86659FD93ULL) ^ ((uint64_t)done * 0xA24BAED4963EE407ULL)
           ^ ((uint64_t)s.turn * 0x9E3779B97F4A7C15ULL);
}

void read_input() {
    if (scanf("%d %d %d %d", &H, &W, &NPK, &MAX_TURN) == 4 && H >= 2) {
        H = min(H, MAXH); W = min(W, MAXW); NPK = min(NPK, MAXPK); MAX_TURN = min(MAX_TURN, MAXTT);
        for (int k = 0; k < NPK; k++) scanf("%d %d %d %d", &SR[k], &SC[k], &DR[k], &DC[k]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 308);
        H = 14; W = 14; NPK = 20; MAX_TURN = 140;
        for (int k = 0; k < NPK; k++) {
            SR[k] = (int)g.next((uint32_t)H); SC[k] = (int)g.next((uint32_t)W);
            DR[k] = (int)g.next((uint32_t)H); DC[k] = (int)g.next((uint32_t)W);
        }
    }
    // ---- 高速化用の前計算 (入力の生成/読み込みには一切影響しない) ----
    for (int i = 0; i < MAXH * MAXW; i++) PK_AT[i] = 0;
    for (int k = 0; k < NPK; k++) {
        PK_AT[SR[k] * MAXW + SC[k]] |= 1u << k;
        SD[k] = abs(SR[k] - DR[k]) + abs(SC[k] - DC[k]);
    }
    for (int k = 0; k < NPK; k++) {
        int b = 1000;
        for (int j = 0; j < NPK; j++) if (j != k) b = min(b, abs(DR[k] - SR[j]) + abs(DC[k] - SC[j]));
        NXT[k] = (b == 1000) ? 0 : b;
    }
    FULLPK = (NPK >= 32) ? 0xFFFFFFFFu : ((1u << NPK) - 1u);
}

void output(const vector<Move> &path) {
    static const char CH[4] = {'U', 'D', 'L', 'R'};
    string r; r.reserve(path.size() * 2);
    for (const Move &mv : path) { r += CH[mv.d]; r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &path) {
    State s; init_state(s);
    float tot = 0.0f;
    for (const Move &mv : path) {
        const int nr = s.r + DX[mv.d], nc = s.c + DY[mv.d];
        if (nr < 0 || nr >= H || nc < 0 || nc >= W) { fprintf(stderr, "[error] 範囲外への移動\n"); return -1.0f; }
        int r, c, carry, deliv; uint32_t picked, done;
        step(s, mv.d, r, c, carry, picked, done, deliv);
        tot += (float)deliv;
        s.r = (int16_t)r; s.c = (int16_t)c; s.carry = (int8_t)carry; s.picked = picked; s.done = done;
    }
    return tot;
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
