// =============================================================================
//  [ビームサーチ系 03] 農場経営 (作物の作付け計画)
// =============================================================================
//  【問題】
//    T ターンの間、K 区画の畑を運営する。所持金の初期値は M。
//    作物は C 種類あり、作物 c は 植える費用 cost_c、成長ターン数 grow_c、収穫収入 rev_c を持つ。
//    毎ターン、次のどちらか 1 つを行う。
//      ・空いている区画 1 つに作物 1 種類を植える (所持金 >= cost_c が必要。所持金が cost_c 減る)
//      ・何もしない
//    植えた作物は grow_c ターン後に自動で収穫され、所持金が rev_c 増えて区画が空く。
//    (T ターン目までに収穫が間に合わない作物は収入にならない)
//    T ターン終了時の所持金を最大化せよ。
//  【入力】
//    T K C M
//    cost_c grow_c rev_c   (C 行)
//  【出力】
//    T 行。そのターンの行動を "区画番号 作物番号"、何もしないなら "-1 -1"。
//  【スコア】 最終所持金 (大きいほど良い)。所持金が負になる行動をしたら 0 点。
//  【入力生成方法】
//    T=120, K=6, C=8, M=200 固定。cost_c は 10..100 の一様整数、grow_c は 3..15 の一様整数、
//    rev_c = round(cost_c * (1.2..2.5 の一様実数))。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「毎ターン 1 アクション」の逐次決定なのでビームサーチが自然。
//     ただしこの問題は、探索アルゴリズムより先に確認すべきことが 2 つある。
//       (1) 合法手をちゃんと全部列挙できているか
//       (2) 状態に「区別する必要のない違い」が入っていないか
//     実際 (1) のミスで区画が毎ターン空転しており、直しただけでスコアが 5 倍になった。
//
//   ● 状態 (State)
//     所持金、各区画の (作物, 残り成長ターン)、ターン。
//     区画は互いに区別できないので、ハッシュは「(作物, 残りターン) の多重集合 + 所持金」から作り、
//     並べ替え違いの状態をまとめて 1 つに潰す。
//
//   ● 手の作り方
//     「収穫が間に合う作物 (turn + grow <= T-1) × 一番若い空き区画」＋「何もしない」。
//     区画は区別できないので、どの空き区画に植えるかを選ぶ必要がない (対称性の除去)。
//     これで候補数が 1/5 になり、重複も消えて実質のビーム幅が大きく増えた。
//
//   ● 評価値
//     「植えた瞬間に利益 (rev - cost) を丸ごと計上したもの」
//      + EVAL_W × (区画ターンの機会費用 PLOT_V) × (残り区画ターン数)
//     ・前半は最終所持金と一致する量。費用だけ先に引く形だと、植えた直後の評価が下がって
//       探索が植えるのを嫌がるので、利益を一括計上する形にしている。
//     ・後半は「区画を 1 ターン使う権利」の値段。これを引かないと
//       「高利益だが回転の遅い作物」に釣られる。PLOT_V は入力から線形計画の双対値として計算する。
//
//   ● 差分計算 / 高速化
//     所持金と区画の状態を更新するだけなので 1 手 O(K)。
//     ハッシュは多重集合なので、変わった区画ぶんだけ XOR し直す。
//
//   ● つまずきポイント
//     ・**enum_moves の所持金判定が「そのターンの収穫を足す前」だった。**
//       採点側は「収穫 → 植える」の順なので、収穫と同じターンに植え直せるのが正しい。
//       合法手を取りこぼしていたため区画が毎ターン空転しており、
//       「ビーム幅をいくら上げてもスコアが 1 の位まで完全に同じ」という症状が出ていた。
//       幅を変えても結果が動かないときは、まず合法手の列挙を疑うこと。
//     ・区画の対称性を潰さないと、同じ状況が K! 通りに散らばってビームが埋まる。
//
//   ● さらに伸ばすなら
//     ・機会費用 PLOT_V をターンごとに変える (終盤は区画が余るので安くなるはず)
//     ・作物の組み合わせを整数計画で解いて上界を出し、どこまで詰められているか測る
//
//  【改善】
//    (1) enum_moves の所持金判定が「そのターンの収穫を足す前」だったため、収穫と同じターンに
//        植え直せず区画が空転していた。収穫分を足してから判定するように修正 (これが最大の要因)。
//    (2) 区画の対称性を除去 (一番若い空き区画にしか植えない) + ハッシュを多重集合化。
//        候補数が 1/5 になり重複も消えたので、実質のビーム幅が大きく増えた。
//    (3) 評価値を「植えた瞬間に利益を全額計上」に変更 (= 真のスコアと一致) し、
//        区画ターンの機会費用 (LP 双対から seed ごとに算出) を先読み項にした。
//    (4) 収穫が間に合わない植え付け (turn + grow > T-1) を候補から除外。BEAM_WIDTH 3000 -> 16000。
//    seed 0..4 合計: 変更前 9320 -> 変更後 49343 (+429.4%)
//
//  【採用したライブラリ】 beam search/1_beam_search
//    実測比較 (seed 0,1,2 の合計スコア): 1_beam_search=5618 / 3_chokudai_search=2635
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
int   BEAM_WIDTH = 16000;  // p1: ビーム幅 (ADAPTIVE_WIDTH=true なら基準値)
float EVAL_W     = 0.8f;   // p2: 区画ターンの機会費用 PLOT_V に掛ける重み

void load_params() {
    pick_env("p1", BEAM_WIDTH);
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
constexpr int  MIN_BEAM_WIDTH = 1;      // ビーム幅の下限
constexpr bool ADAPTIVE_WIDTH = true;   // 残り時間からビーム幅を自動調整する (時間を使い切る)
constexpr bool USE_HASH_DEDUP = true;   // ハッシュによる重複除去を行う

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_expanded = 0, g_cand_total = 0;
static int g_turn_done = 0;
static float g_best_eval = 0.0f;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (農場経営)
// #############################################################################
constexpr int MAXK = 12, MAXCR = 16, MAXTT = 400;
constexpr int MAX_BRANCH = MAXCR + 1;         // ★区画の対称性を潰したので「作物数 + 何もしない」だけ
constexpr int BEAM_CAP   = 32768;             // ★ビーム幅の上限

int MAX_TURN, K, NC, M0;
static int CCOST[MAXCR], CGROW[MAXCR], CREV[MAXCR];
static float CPROF[MAXCR];                    // 利益 = rev - cost (必ず正)
static float PLOT_V = 0.0f;                   // 区画 1 ターンあたりの価値 (LP 双対から算出)
static uint64_t ZC[MAXCR + 2][64], ZM[8192];

struct Move { int8_t plot, crop; };   // plot < 0 なら何もしない

struct State {
    float    score;            // 評価値 (= 確定した最終所持金の見込み)
    uint64_t hash;
    float    money;            // 実際の所持金 (植えられるかの判定に使う)
    int32_t  turn;
    int8_t   crop[MAXK];       // -1 は空き
    int8_t   left[MAXK];       // 収穫までの残りターン
};

// 区画は互いに区別できないので、ハッシュは (作物, 残り) の多重集合として作る (加算 = 順序非依存)
static inline uint64_t plots_hash(const State &s) {
    uint64_t h = 0;
    for (int k = 0; k < K; k++) h += ZC[s.crop[k] + 1][s.left[k] < 64 ? s.left[k] : 63];
    return h;
}
static inline uint64_t mix_hash(uint64_t ph, float money, int turn) {
    const int mn = (int)min(8191.0f, max(0.0f, money));
    return ph ^ ZM[mn] ^ ((uint64_t)turn * 0x9E3779B97F4A7C15ULL);
}

void init_state(State &s) {
    s.score = (float)M0; s.money = (float)M0; s.turn = 0;
    for (int k = 0; k < K; k++) { s.crop[k] = -1; s.left[k] = 0; }
    s.hash = mix_hash(plots_hash(s), s.money, 0);
}

// ★区画は区別できないので「一番若い空き区画」にしか植えない (対称性の除去)
//   さらに turn + grow <= MAX_TURN - 1 でないと収穫が間に合わない (植え損になる)
inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= MAX_TURN) return 0;
    // ★このターンに収穫される分は「植える前」に入るので、所持金に足してから判定する。
    //   ここを忘れると収穫と同じターンに植え直せず、区画が 1 ターンずつ空転する (これが致命的だった)。
    float avail = s.money;
    int fe = -1;
    for (int k = 0; k < K; k++) {
        if (s.crop[k] < 0) { if (fe < 0) fe = k; continue; }
        if (s.left[k] == 1) { avail += (float)CREV[s.crop[k]]; if (fe < 0) fe = k; }
    }
    int m = 0;
    if (fe >= 0) {
        const int rem = MAX_TURN - 1 - s.turn;
        for (int c = 0; c < NC; c++)
            if (CGROW[c] <= rem && (float)CCOST[c] <= avail) { out[m].plot = (int8_t)fe; out[m].crop = (int8_t)c; m++; }
    }
    out[m].plot = -1; out[m].crop = -1; m++;      // 何もしない
    return m;
}

// このターンに自動収穫される収入
static inline float harvest_of(const State &s) {
    float g = 0.0f;
    for (int k = 0; k < K; k++) if (s.crop[k] >= 0 && s.left[k] == 1) g += (float)CREV[s.crop[k]];
    return g;
}

// ★評価値の作り方
//   enum_moves の制限により「植えた作物は必ず収穫される」ので、
//   最終所持金 = M0 + Σ(rev - cost) が厳密に成り立つ。
//   そこで「植えた瞬間に利益を丸ごと計上する」ことで、評価値 = 真のスコアになり、
//   探索が投資の途中でスコアが凹むのを気にしなくて済む (これが一番効いた)。
//   さらに EVAL_W で「区画の回転の速さ (残り成長ターンの合計)」を減点し、
//   長期作物で区画を塞ぎ続ける手を抑える先読み項にしている。
inline float calc_score(const State &s, const Move &mv) {
    const int t = s.turn;
    float cap0 = 0.0f, cap1 = 0.0f;
    for (int k = 0; k < K; k++) {
        const int a = MAX_TURN - t - s.left[k];            if (a > 0) cap0 += (float)a;
        int nl = s.left[k] > 0 ? s.left[k] - 1 : 0;
        if (mv.plot == k) nl = CGROW[mv.crop];
        const int b = MAX_TURN - (t + 1) - nl;             if (b > 0) cap1 += (float)b;
    }
    const float prof = (mv.plot < 0) ? 0.0f : CPROF[mv.crop];
    return prof + EVAL_W * PLOT_V * (cap1 - cap0);
}

inline uint64_t calc_hash(const State &s, const Move &mv) {
    uint64_t h = 0;
    float money = s.money + harvest_of(s);
    for (int k = 0; k < K; k++) {
        int cr = s.crop[k], lf = s.left[k];
        if (cr >= 0) { if (--lf <= 0) { cr = -1; lf = 0; } }
        if (mv.plot == k) { cr = mv.crop; lf = CGROW[mv.crop]; }
        h += ZC[cr + 1][lf < 64 ? lf : 63];
    }
    if (mv.plot >= 0) money -= (float)CCOST[mv.crop];
    return mix_hash(h, money, s.turn + 1);
}

inline void apply_move(State &s, const Move &mv) {
    for (int k = 0; k < K; k++) if (s.crop[k] >= 0) {
        if (--s.left[k] <= 0) { s.money += (float)CREV[s.crop[k]]; s.crop[k] = -1; s.left[k] = 0; }
    }
    if (mv.plot >= 0) { s.money -= (float)CCOST[mv.crop]; s.crop[mv.plot] = mv.crop; s.left[mv.plot] = (int8_t)CGROW[mv.crop]; }
    s.turn++;
    s.hash = mix_hash(plots_hash(s), s.money, s.turn);
}

void read_input() {
    if (scanf("%d %d %d %d", &MAX_TURN, &K, &NC, &M0) == 4 && K >= 1) {
        MAX_TURN = min(MAX_TURN, MAXTT); K = min(K, MAXK); NC = min(NC, MAXCR);
        for (int c = 0; c < NC; c++) scanf("%d %d %d", &CCOST[c], &CGROW[c], &CREV[c]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 303);
        MAX_TURN = 120; K = 6; NC = 8; M0 = 200;
        for (int c = 0; c < NC; c++) {
            CCOST[c] = 10 + (int)g.next(91);
            CGROW[c] = 3 + (int)g.next(13);
            CREV[c] = (int)lroundf((float)CCOST[c] * (1.2f + 1.3f * g.nextf()));
        }
    }
    for (int c = 0; c < NC; c++) CPROF[c] = (float)(CREV[c] - CCOST[c]);
    // ---- 区画ターンの影の価格 PLOT_V を LP 双対から求める ----
    //   主問題: max Σ n_c*prof_c  s.t. Σ n_c <= T (毎ターン 1 手),  Σ n_c*grow_c <= K*T (区画ターン)
    //   双対  : min T*u + K*T*v   s.t. u + grow_c*v >= prof_c,  u,v >= 0
    //   最適な v が「区画を 1 ターン塞ぐことの機会費用」になる。作物構成が seed ごとに違うので
    //   固定値ではなく入力から計算する (これが無いと高利益・低回転の作物に釣られる)。
    {
        const double TT = (double)MAX_TURN, KK = (double)K;
        double bu = 0.0, bv = 0.0, bobj = 1e18;
        auto feas = [&](double u, double v) {
            if (u < -1e-9 || v < -1e-9) return false;
            for (int c = 0; c < NC; c++) if (u + (double)CGROW[c] * v < (double)CPROF[c] - 1e-6) return false;
            return true;
        };
        auto tryuv = [&](double u, double v) {
            if (!feas(u, v)) return;
            const double o = TT * u + KK * TT * v;
            if (o < bobj) { bobj = o; bu = u; bv = v; }
        };
        double mp = 0.0, mr = 0.0;
        for (int c = 0; c < NC; c++) { mp = max(mp, (double)CPROF[c]); mr = max(mr, (double)CPROF[c] / (double)CGROW[c]); }
        tryuv(mp, 0.0);
        tryuv(0.0, mr);
        for (int a = 0; a < NC; a++) for (int b = 0; b < NC; b++) {
            if (CGROW[a] == CGROW[b]) continue;
            const double v = ((double)CPROF[a] - CPROF[b]) / ((double)CGROW[a] - CGROW[b]);
            tryuv((double)CPROF[a] - (double)CGROW[a] * v, v);
        }
        (void)bu;
        PLOT_V = (float)bv;
    }
    Xor128 z; z.seed(4649);
    for (int c = 0; c <= MAXCR + 1; c++) for (int l = 0; l < 64; l++) ZC[c][l] = ((uint64_t)z.next() << 32) | z.next();
    for (int i = 0; i < 8192; i++) ZM[i] = ((uint64_t)z.next() << 32) | z.next();
}

void output(const vector<Move> &path) {
    string r; r.reserve(path.size() * 6);
    for (const Move &mv : path) { r += to_string((int)mv.plot); r += ' '; r += to_string((int)mv.crop); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &path) {
    float money = (float)M0;
    int crop[MAXK], left[MAXK];
    for (int k = 0; k < K; k++) { crop[k] = -1; left[k] = 0; }
    for (int t = 0; t < (int)path.size(); t++) {
        for (int k = 0; k < K; k++) if (crop[k] >= 0) { if (--left[k] <= 0) { money += (float)CREV[crop[k]]; crop[k] = -1; } }
        const int p = path[t].plot, c = path[t].crop;
        if (p >= 0) {
            if (p >= K || crop[p] >= 0 || c < 0 || c >= NC || money < (float)CCOST[c]) { fprintf(stderr, "[error] 不正な行動 (t=%d)\n", t); return -1.0f; }
            money -= (float)CCOST[c]; crop[p] = c; left[p] = CGROW[c];
        }
    }
    return money;
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
