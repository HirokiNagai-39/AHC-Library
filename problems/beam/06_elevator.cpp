// =============================================================================
//  [ビームサーチ系 06] エレベーター運行 (待ち時間最小化)
// =============================================================================
//  【問題】
//    F 階建てのビルにエレベーターが 1 台あり、定員は Q 人。時刻 0 に 0 階にいる。
//    乗客は N 人いて、乗客 i は時刻 t_i に階 f_i に現れ、階 g_i へ行きたい (すべて事前に分かっている)。
//    毎ターン、次のいずれかを行う。
//      ・上へ 1 階移動 / 下へ 1 階移動 / その階で停止 (乗降)
//    停止すると、まず現在の階が目的地の乗客が全員降り、次にその階で待っている乗客が
//    定員まで (番号の小さい順に) 乗る。
//    スコア = Σ_i (乗客 i が降りた時刻 - t_i)。T ターンまでに降りられなかった乗客は
//    (T - t_i) + 100 のペナルティとする。これを最小化せよ。
//  【入力】
//    F Q N T
//    t_i f_i g_i   (N 行)
//  【出力】
//    T 行。各ターンの行動 "U" / "D" / "S"。
//  【スコア】 上記の合計 (小さいほど良い)。
//  【入力生成方法】
//    F=10, Q=4, N=40, T=200 固定。t_i は [0,T*0.7] の一様整数、f_i,g_i は 0..F-1 の一様整数 (f_i != g_i)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     1 手の選択肢が 3 通りしかない代わりに手数が長い、典型的なビームサーチ問題。
//     この問題でいちばん大事なのは**スコアの持ち方**と**評価値に余計な項を入れないこと**。
//     待ち時間の総和は「毎ターン、まだ降りていない乗客の人数を足したもの」に等しい。
//     この形にすると 1 手の差分が O(1) になり、しかも真のスコアと完全に一致する。
//
//   ● 状態 (State)
//     (現在階, 乗車中の乗客ビット, 降車済みビット, ターン)。
//     乗客 40 人なので 64bit 整数 2 本で足りる。State が小さいほどビーム幅を取れる。
//
//   ● 手の作り方
//     上へ / 下へ / 停止 の 3 通り。停止すると「目的階の客が降りる → 待っている客が定員まで乗る」。
//
//   ● 評価値
//     真のスコアそのもの (EVAL_W = 0)。
//     **ここが最大の落とし穴**: 初版は「- EVAL_W × 乗車中の人数」を先読み項として足していた。
//     ところが乗車中の客も未配達なので毎ターン +1 のコストが乗る。EVAL_W = 1.0 だとこれが
//     ちょうど相殺し、「乗せたまま降ろさずに走り回る」のが評価上ゼロコストになってしまう。
//     探索が強くなるほどこの抜け穴を悪用するので、幅を上げても良くならない。0 にすると -32% 改善した。
//     **先読み項を足すときは「終端でどうなるか」と「他の項と相殺しないか」を必ず確認すること。**
//
//   ● 差分計算 / 高速化
//     乗降処理を素朴に書くと乗客数 N のループになるが、
//       ・階ごとの乗客ビットマスク SRC_MASK[階] / DST_MASK[階]
//       ・時刻ごとの「もう現れた乗客」ビットマスク ARRIVED[t]
//     を前計算しておけば、乗降も待ち人数も全部ビット演算 O(1) で済む。
//     探索が約 3 倍速くなり、1.9 秒使い切っていたものが 0.3 秒で飽和するようになった。
//
//   ● つまずきポイント
//     ・上記の「評価項が相殺して抜け穴になる」問題。
//     ・定員まで乗せる順序 (番号の小さい順) は採点側と一致させること。ここがずれると
//       出力の再生結果が変わる。
//
//   ● さらに伸ばすなら
//     ・幅 8192 で飽和しているので、この定式化では上限に近い
//     ・伸ばすなら乗客数を増やす (ビットマスクが 64 を超えると設計が変わる) 方向
//
//  【改善】
//    (1) 評価値の EVAL_W を 1.0 -> 0.0 に。EVAL_W=1.0 だと「乗車中の客」の 1 ターンあたりの
//        コストが 0 になり、探索が強くなるほど「乗せたまま降ろさない」解を選んでしまっていた。
//    (2) step()/待ち人数を全部ビットマスクの前計算で O(1) 化 (探索は約 3 倍速。実行 1.9s -> 0.3s)。
//    seed 0..4 合計: 変更前 4826 -> 変更後 3283 (-32.0%)   ※小さいほど良い
//
//  【採用したライブラリ】 beam search/1_beam_search
//    実測比較 (seed 0,1,2 の合計スコア): 1_beam_search=2903 / 3_chokudai_search=3692
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
float EVAL_W     = 0.0f;   // p2: 乗車中の人数を評価から引く重み (0 が最良だった)
float EVAL_D     = 0.0f;   // p3: 乗車中の客の目的階までの距離の和のペナルティ重み
float EVAL_P     = 0.0f;   // p4: 1 人乗せるごとのボーナス (早い乗車を促す先読み項)

void load_params() {
    pick_env("p1", BEAM_WIDTH);
    pick_env("p2", EVAL_W);
    pick_env("p3", EVAL_D);
    pick_env("p4", EVAL_P);
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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (エレベーター運行)
// #############################################################################
constexpr int MAXF = 16, MAXNP = 64, MAXTT = 400;
constexpr int MAX_BRANCH = 3;
constexpr int BEAM_CAP   = 32768;          // 幅 8192 で既に飽和するのでこれで十分

int NF, QCAP, NP, MAX_TURN;
static int PT[MAXNP], PF[MAXNP], PG[MAXNP];
static uint64_t ZF[MAXF], ZT[MAXTT + 1];
// ---- 前計算 (ここが速度の肝) ------------------------------------------------
static uint64_t SRC_MASK[MAXF];            // その階から乗る客のビット集合
static uint64_t DST_MASK[MAXF];            // その階で降りる客のビット集合
static uint64_t ARRIVED[MAXTT + 2];        // 時刻 t までに現れた客のビット集合
static int      NARRIVED[MAXTT + 2];       // その個数

struct Move { int8_t a; };      // 0:上 1:下 2:停止

struct State {
    float    score;
    uint64_t hash;
    uint64_t on, done;          // 乗車中 / 降車済み
    int32_t  turn;
    int8_t   fl;
};

static inline uint64_t mix64(uint64_t x) {
    x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27; x *= 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

void init_state(State &s) {
    s.score = 0.0f; s.turn = 0; s.fl = 0; s.on = 0; s.done = 0;
    s.hash = ZF[0];
}

inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= MAX_TURN) return 0;
    int m = 0;
    if (s.fl + 1 < NF) out[m++].a = 0;
    if (s.fl > 0)      out[m++].a = 1;
    out[m++].a = 2;
    return m;
}

// 行動 a を取った後の (階, 乗車中, 降車済み) を求める  ★全部ビット演算で O(1)
static inline void step(const State &s, int a, int &fl, uint64_t &on, uint64_t &done) {
    fl = s.fl; on = s.on; done = s.done;
    if (a == 0) { fl++; return; }
    if (a == 1) { fl--; return; }
    const uint64_t drop = on & DST_MASK[fl];      // 目的階に着いた客は全員降りる
    on &= ~drop; done |= drop;
    int cap = QCAP - __builtin_popcountll(on);
    if (cap > 0) {
        uint64_t w = SRC_MASK[fl] & ARRIVED[s.turn] & ~on & ~done;   // 待っている客
        while (cap > 0 && w) { const uint64_t b = w & (~w + 1); on |= b; w ^= b; cap--; }  // 番号の小さい順
    }
}

inline float calc_score(const State &s, const Move &mv) {
    int fl; uint64_t on, done;
    step(s, mv.a, fl, on, done);
    // 次のターン開始時点で「現れていて、まだ降りていない」乗客の数 = 1 ターンぶんの待ち時間
    // done は必ず ARRIVED[turn] の部分集合なので引き算だけで求まる (元の O(N) ループを O(1) 化)
    const int wait = NARRIVED[s.turn + 1] - __builtin_popcountll(done);
    float v = (float)wait - EVAL_W * (float)__builtin_popcountll(on);
    if (EVAL_D != 0.0f) {            // 乗せている客の目的階から遠いほど損、という勾配
        int d = 0;
        for (uint64_t m = on; m; m &= m - 1) d += abs(fl - PG[__builtin_ctzll(m)]);
        v += EVAL_D * (float)d;
    }
    if (EVAL_P != 0.0f)              // 早く乗せた解ほど prefix スコアが下がる = 先読みになる
        v -= EVAL_P * (float)__builtin_popcountll(on & ~s.on);
    return v;
}
inline uint64_t calc_hash(const State &s, const Move &mv) {
    int fl; uint64_t on, done;
    step(s, mv.a, fl, on, done);
    return ZF[fl] ^ mix64(on * 0x9E3779B97F4A7C15ULL + 1) ^ mix64(done) ^ ZT[s.turn + 1];
}
inline void apply_move(State &s, const Move &mv) {
    int fl; uint64_t on, done;
    step(s, mv.a, fl, on, done);
    s.fl = (int8_t)fl; s.on = on; s.done = done; s.turn++;
    s.hash = ZF[fl] ^ mix64(on * 0x9E3779B97F4A7C15ULL + 1) ^ mix64(done) ^ ZT[s.turn];
}

void read_input() {
    if (scanf("%d %d %d %d", &NF, &QCAP, &NP, &MAX_TURN) == 4 && NF >= 2) {
        NF = min(NF, MAXF); NP = min(NP, MAXNP); MAX_TURN = min(MAX_TURN, MAXTT);
        for (int i = 0; i < NP; i++) scanf("%d %d %d", &PT[i], &PF[i], &PG[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 306);
        NF = 10; QCAP = 4; NP = 40; MAX_TURN = 200;
        for (int i = 0; i < NP; i++) {
            PT[i] = (int)g.next((uint32_t)(MAX_TURN * 7 / 10));
            PF[i] = (int)g.next((uint32_t)NF);
            do { PG[i] = (int)g.next((uint32_t)NF); } while (PG[i] == PF[i]);
        }
    }
    Xor128 z; z.seed(90210);
    for (int i = 0; i < MAXF; i++) ZF[i] = ((uint64_t)z.next() << 32) | z.next();
    for (int i = 0; i <= MAXTT; i++) ZT[i] = ((uint64_t)z.next() << 32) | z.next();
    // ---- 高速化用の前計算 (入力の生成/読み込みには一切影響しない) ----
    for (int f = 0; f < MAXF; f++) { SRC_MASK[f] = 0; DST_MASK[f] = 0; }
    for (int i = 0; i < NP; i++) { SRC_MASK[PF[i]] |= 1ULL << i; DST_MASK[PG[i]] |= 1ULL << i; }
    uint64_t acc = 0;
    for (int t = 0; t <= MAX_TURN + 1; t++) {
        for (int i = 0; i < NP; i++) if (PT[i] == t) acc |= 1ULL << i;
        ARRIVED[t] = acc; NARRIVED[t] = __builtin_popcountll(acc);
    }
}

void output(const vector<Move> &path) {
    string r; r.reserve(path.size() * 2);
    static const char C[3] = {'U', 'D', 'S'};
    for (const Move &mv : path) { r += C[mv.a]; r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &path) {
    State s; init_state(s);
    static int outt[MAXNP];
    for (int i = 0; i < NP; i++) outt[i] = -1;
    for (int t = 0; t < (int)path.size() && t < MAX_TURN; t++) {
        int fl; uint64_t on, done;
        step(s, path[t].a, fl, on, done);
        for (int i = 0; i < NP; i++) if (((done >> i) & 1) && outt[i] < 0) outt[i] = t + 1;
        s.fl = (int8_t)fl; s.on = on; s.done = done; s.turn++;
    }
    float tot = 0.0f;
    for (int i = 0; i < NP; i++) tot += (outt[i] >= 0) ? (float)(outt[i] - PT[i]) : (float)(MAX_TURN - PT[i]) + 100.0f;
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
