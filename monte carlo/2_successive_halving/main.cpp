// https://github.com/HirokiNagai-39/AHC-Library
// =============================================================================
//  モンテカルロ法 + 逐次絞り込み (Successive Halving)   ---  AHC 用 高速テンプレート
// =============================================================================
//  フェーズごとに下位候補を切り捨て、有望な候補にシナリオを集中させる。同じ時間でより深く比較できる。
//
//  【この手法が向く問題】
//    未来が確率的に決まる問題 (何が来るか分からない / 相手の行動が分からない 等)。
//    「今この手を選んだら、最終的に平均でどれくらいのスコアになるか」を
//    ランダムな未来(シナリオ)を何本も試して見積もり、期待値が最良の手を選ぶ。
//
//  【★シナリオを公平にするための 4 原則★】  ← ここが最重要
//    1. 1 ラウンドで作ったシナリオは「全候補」で使い回す (共通乱数法)。
//       候補ごとに別のシナリオを引くと、運の良いシナリオを引いた候補が勝ってしまう。
//       同じ未来で比べると差だけが残るので、少ないシナリオ数でも正しく比較できる。
//    2. シナリオは「ターン番号で引ける配列」にする。乱数を消費順に引くと、
//       候補によって行動が変わった瞬間に未来そのものが変わってしまう。
//    3. rollout の中で方策がランダムに動く場合、その乱数も候補間で同じ種から始める。
//    4. 時間切れの打ち切りは必ず「ラウンド境界」で行う。ラウンドの途中で切ると
//       候補ごとにシナリオ数が変わり、比較が壊れる。
//    ※ 全候補のシナリオ数が必ず等しくなるので、平均を取らず「合計」のまま比較できる(除算不要)。
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct Move     { ... };                  ... 手
//    struct State    { float score; ... };     ... 状態 (score は必須)
//    struct Scenario { ... };                  ... 未来の不確定要素 1 本ぶん
//    init_state(State&)                        ... 初期状態
//    enum_moves(const State&, Move*)           ... 今の候補手を全列挙して個数を返す
//    calc_score(const State&, Move)            ... その手で確定するスコア差分
//    apply_move(State&, Move)                  ... 手を適用する (score は触らない)
//    reveal_turn(State&, int t)                ... ターン t で初めて分かる情報を state に入れる
//    gen_scenario(Scenario&, Xor128&, int)     ... シナリオを 1 本作る
//    rollout(State&, const Scenario&, Xor128&) ... 最後までシミュレートして最終スコアを返す
//
//  【高速化のポイント】
//    ・シナリオは 1 ラウンドに 1 本だけ作り、全候補で共有する (生成コストを候補数で割れる)
//    ・候補を適用した直後の状態を 1 回だけ作り置きし、rollout ごとにそこからコピーする
//    ・シナリオ数は時間から自動決定。1 ラウンドの実測時間で「次が入るか」を判定して打ち切る
//    ・期待値の集計は double (float だと数万回の加算で精度が落ちる)
//    ・時間計測は rdtsc/cntvct 直読み、1 ラウンドに 1 回だけ
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
float ROLLOUT_TH  = 1.0f;  // p1: rollout の方策の閾値 (価値密度 v/s がこれ以上なら詰める。デモ用)
float TIME_ALPHA  = 0.0f;  // p2: ターンごとの時間配分 (0=均等, 大きいほど序盤に厚く配分)
int   HALVE_RATE  = 2;     // p3: 1 フェーズごとに候補を 1/HALVE_RATE に絞る
int   MAX_SCENARIO = 1 << 30;  // 1 ターンで回すシナリオ数の上限 (既定は実質無制限)

void load_params() {
    pick_env("p1", ROLLOUT_TH);
    pick_env("p2", TIME_ALPHA);
    pick_env("p3", HALVE_RATE);
    pick_env("MAX_SCENARIO", MAX_SCENARIO);   // 比較用: MAX_SCENARIO=1 にすると貪欲法相当になる
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化のとき こちらを有効化 (デモは最大化)
// constexpr bool MAXIMIZE = false;    // ← スコア最小化のとき こちらを有効化

inline bool is_better(double a, double b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }
constexpr double WORST_SCORE = MAXIMIZE ? -1e300 : 1e300;

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS = 1900.0f;   // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr float ROUND_MARGIN  = 1.10f;     // 「次のラウンドが入るか」の判定に掛ける安全係数
// (この手法は探索の設定より「rollout をどれだけ軽くできるか」が効くので、ここは少なめ)

// 統計 (デバッグ用。不要なら消して良い)
static ll  g_rounds = 0, g_rollouts = 0;
static int g_min_sc = INT32_MAX, g_max_sc = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■
// #     (デモ: オンライン箱詰め / 未来のアイテムが分からない状態で詰める)
// #       K 個の箱(容量 CAP)がある。T ターン、毎ターン 1 個アイテム(大きさ s, 価値 v)が来る。
// #       どれかの箱に入れる(容量が足りる箱のみ)か、捨てる。入れた価値の合計を最大化する。
// #       ★未来のアイテムは分からない。分かるのは分布 (s は 1..SMAX, v は 1..VMAX の一様乱数) だけ。
// #       標準入力が無ければランダムに問題を生成するので、そのまま実行できる。
// #############################################################################
constexpr int MAXK = 16;              // 箱の数の上限
constexpr int MAXT = 1000;            // ターン数の上限
constexpr int MAX_CAND = MAXK + 1;    // ★1 ターンの候補手の最大数 (配列サイズに効く)

int K, CAP, T, SMAX, VMAX;            // 入力 (分布 SMAX, VMAX は解答側も知っている情報)

// ---- ジャッジ側だけが知っている「本当の未来」 ----
// ★ turn に到達するまで参照してはいけない。必ず reveal() 経由で読むこと。
static int TRUE_S[MAXT], TRUE_V[MAXT];
// (State の定義後に reveal_turn を定義する)

// ★必須: 手
struct Move { int8_t box; };          // 0..K-1: その箱に入れる / -1: 捨てる

// ★必須: 状態。float score を必ず持たせる。
struct State {
    float   score;                    // ★必須: 確定しているスコア
    int32_t turn;                     // 今が何ターン目か
    int32_t cur_s, cur_v;             // このターンに来たアイテム (reveal 済み)
    int16_t rest[MAXK];               // 各箱の残り容量
};

// ★必須: 未来の不確定要素 1 本ぶん。
//   ★ターン番号で引ける「配列」にすること。乱数を消費順に引く実装にすると、
//     候補ごとに行動が変わった瞬間に未来まで変わってしまい、比較が不公平になる。
struct Scenario { uint8_t s[MAXT], v[MAXT]; };

// ★必須: turn になって初めて分かる情報を state に入れる
inline void reveal_turn(State &s, int turn) { s.cur_s = TRUE_S[turn]; s.cur_v = TRUE_V[turn]; }

// ★必須: 初期状態
void init_state(State &s) {
    s.score = 0.0f;
    s.turn  = 0;
    s.cur_s = s.cur_v = 0;
    for (int k = 0; k < K; k++) s.rest[k] = (int16_t)CAP;
}

// ★必須: 今の候補手を全部 out[] に詰めて個数を返す
inline int enum_moves(const State &s, Move *out) {
    int m = 0;
    for (int k = 0; k < K; k++) if (s.rest[k] >= s.cur_s) out[m++].box = (int8_t)k;
    out[m++].box = -1;                // 捨てる (常に可能)
    return m;
}

// ★必須: その手で確定するスコア差分
inline float calc_score(const State &s, const Move &mv) { return mv.box < 0 ? 0.0f : (float)s.cur_v; }

// ★必須: 手を適用する (score は触らない。フレームワークが calc_score を足す)
inline void apply_move(State &s, const Move &mv) {
    if (mv.box >= 0) s.rest[mv.box] -= (int16_t)s.cur_s;
    s.turn++;
}

// ★必須: シナリオを 1 本作る (from 以降のターンぶんだけで良い)
inline void gen_scenario(Scenario &sc, Xor128 &rnd, int from) {
    for (int t = from; t < T; t++) {
        sc.s[t] = (uint8_t)(1 + rnd.next((uint32_t)SMAX));
        sc.v[t] = (uint8_t)(1 + rnd.next((uint32_t)VMAX));
    }
}

// ★必須: s の状態から最後までシミュレートして「最終スコア」を返す。
//   s は作業用コピーなので好きに壊して良い。速さが全てなので、方策は軽いものにする。
//   デモの方策: 価値密度 v/s が ROLLOUT_TH 以上なら、詰めた後の余りが最小の箱へ (best fit)
inline float rollout(State &s, const Scenario &sc, Xor128 &rnd) {
    (void)rnd;                        // 方策が乱数を使う場合はこれを使う (候補間で同じ種から始まる)
    float total = s.score;
    for (int t = s.turn; t < T; t++) {
        const int si = sc.s[t], vi = sc.v[t];
        if ((float)vi < ROLLOUT_TH * (float)si) continue;      // 価値が薄いので捨てる
        int best = -1, waste = INT32_MAX;
        for (int k = 0; k < K; k++) {
            const int r = s.rest[k] - si;
            if (r >= 0 && r < waste) { waste = r; best = k; }
        }
        if (best >= 0) { s.rest[best] -= (int16_t)si; total += (float)vi; }
    }
    return total;
}

// 入力 (デモ: 標準入力が無ければランダム生成)
void read_input() {
    if (scanf("%d %d %d %d %d", &K, &CAP, &T, &SMAX, &VMAX) == 5 && K >= 1) {
        K = min(K, MAXK); T = min(T, MAXT);
        for (int t = 0; t < T; t++) scanf("%d %d", &TRUE_S[t], &TRUE_V[t]);
    } else {
        K = 8; CAP = 100; T = 200; SMAX = 30; VMAX = 100;
        Xor128 g; g.seed(20260824);
        for (int t = 0; t < T; t++) {
            TRUE_S[t] = 1 + (int)g.next((uint32_t)SMAX);
            TRUE_V[t] = 1 + (int)g.next((uint32_t)VMAX);
        }
    }
}

// 出力 (各ターンに選んだ箱。-1 は捨てた)
void output(const vector<Move> &hist) {
    string res;
    res.reserve(hist.size() * 3);
    for (const Move &mv : hist) { res += to_string((int)mv.box); res += '\n'; }
    fputs(res.c_str(), stdout);
}

// 出力を再生して「真のスコア」を計算する (実装のバグ検出用)
float replay_true_score(const vector<Move> &hist) {
    State s; init_state(s);
    float total = 0.0f;
    for (int t = 0; t < (int)hist.size(); t++) {
        const int si = TRUE_S[t], vi = TRUE_V[t];
        const int b = hist[t].box;
        if (b < 0) continue;
        if (b >= K || s.rest[b] < si) { fprintf(stderr, "[error] 不正な手 (turn=%d)\n", t); return -1.0f; }
        s.rest[b] -= (int16_t)si;
        total += (float)vi;
    }
    return total;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
//  ターンごとの時間配分
// =============================================================================
//  TIME_ALPHA = 0 なら全ターン均等。大きくすると序盤に多く時間を割り当てる。
//  各ターンの締切は「累積」で持つので、早く終わったターンの余りは自動的に後ろへ回る。
static float g_deadline[MAXT + 1];
static void plan_time(float begin_ms, float total_ms) {
    double sum = 0.0;
    for (int t = 0; t < T; t++) sum += pow((double)(T - t) / (double)T, (double)TIME_ALPHA);
    double acc = 0.0;
    for (int t = 0; t < T; t++) {
        acc += pow((double)(T - t) / (double)T, (double)TIME_ALPHA);
        g_deadline[t] = begin_ms + total_ms * (float)(acc / sum);
    }
}

// ---- 1 ターンぶんの作業領域 (静的確保。探索中に malloc しない) ----
static Move     g_cand[MAX_CAND];      // 候補手
static State    g_base[MAX_CAND];      // 候補を適用した直後の状態 (作り置き)
static double   g_sum [MAX_CAND];      // 候補ごとのスコア合計 (float だと精度が落ちる)
static State    g_work;                // rollout 用の作業状態
static Scenario g_sc;                  // ★シナリオは 1 本だけ持ち、全候補で共有する
static uint32_t g_round_id = 0;        // シナリオの種 (ターンをまたいで別の未来を引く)

// =============================================================================
// 7. モンテカルロ法 + 逐次絞り込み (Successive Halving) 本体
// =============================================================================
//  弱い候補にシナリオを使い続けるのは無駄なので、フェーズごとに下位を切り捨てて
//  生き残った候補にシナリオを集中させる。同じ時間でより多くのシナリオを有望手に回せる。
//
//  ★公平性: 1 つのフェーズの中では「生き残っている全候補」が必ず同じシナリオを試す。
//    生き残り候補どうしは常に同じシナリオ集合で比較されるので、比較は壊れない。
//    (途中で落ちた候補はシナリオ数が少ないが、落とす時点では全員同数で比べている)
Move choose_move(const State &state, float begin_ms, float deadline_ms) {
    const int C = enum_moves(state, g_cand);
    if (C <= 1) { g_min_sc = 0; return g_cand[0]; }

    for (int c = 0; c < C; c++) {
        g_base[c] = state;
        g_base[c].score += calc_score(state, g_cand[c]);
        apply_move(g_base[c], g_cand[c]);
        g_sum[c] = 0.0;
    }

    // 生存候補の添字リスト
    static int32_t alive_id[MAX_CAND];
    for (int c = 0; c < C; c++) alive_id[c] = c;
    int alive = C;

    // 何フェーズに分けるか (毎フェーズで 1/HALVE_RATE に絞る)
    int phases = 1;
    for (int c = C; c > 1;) { c = max(1, (c + HALVE_RATE - 1) / HALVE_RATE); phases++; }

    int   n     = 0;
    float prev  = timer.ms();
    float round = 0.0f;
    const float span = deadline_ms - begin_ms;

    for (int ph = 0; ph < phases; ph++) {
        const float ph_deadline = begin_ms + span * (float)(ph + 1) / (float)phases;
        // --- このフェーズのシナリオを回す ---
        while (n < MAX_SCENARIO) {
            if (n > 0 && prev + round * ROUND_MARGIN > ph_deadline) break;
            Xor128 rs; rs.seed(g_round_id++);
            gen_scenario(g_sc, rs, state.turn + 1);
            Xor128 rp; rp.seed(0x9E3779B9u ^ g_round_id);
            for (int i = 0; i < alive; i++) {           // ★生存候補は全員同じシナリオ
                const int c = alive_id[i];
                g_work = g_base[c];
                Xor128 r = rp;
                g_sum[c] += (double)rollout(g_work, g_sc, r);
            }
            n++; g_rollouts += alive;
            const float now = timer.ms();
            round = now - prev;
            prev  = now;
        }
        if (alive <= 1) break;
        // --- 上位だけ残す (全ソートせず nth_element) ---
        const int keep = max(1, (alive + HALVE_RATE - 1) / HALVE_RATE);
        nth_element(alive_id, alive_id + keep, alive_id + alive,
                    [](int32_t a, int32_t b) { return is_better(g_sum[a], g_sum[b]); });
        alive = keep;
        if (prev >= deadline_ms) break;
    }
    g_rounds += n;
    g_min_sc = min(g_min_sc, n); g_max_sc = max(g_max_sc, n);

    int best = alive_id[0];
    for (int i = 1; i < alive; i++) if (is_better(g_sum[alive_id[i]], g_sum[best])) best = alive_id[i];
    return g_cand[best];
}

// =============================================================================
// 8. main
// =============================================================================
int main() {
    timer.start();      // ★ 一番最初にタイマー開始
    load_params();      // 環境変数からパラメータを読む (optuna 用)
    read_input();

    static State state;
    init_state(state);
    plan_time(timer.ms(), TIME_LIMIT_MS - timer.ms());   // ターンごとの締切を決める

    vector<Move> hist(T);
    for (int t = 0; t < T; t++) {
        reveal_turn(state, t);              // ★このターンに初めて分かる情報を state に取り込む
        const Move mv = choose_move(state, t == 0 ? timer.ms() : g_deadline[t - 1], g_deadline[t]);
        hist[t] = mv;
        state.score += calc_score(state, mv);
        apply_move(state, mv);
    }
    // ★最終ターンの実現値を state に反映する。これが無いと下の [check] の running 側が
    //   最後の 1 ターンぶんだけ足りない値になり、差分計算のバグと区別できなくなる。
    if (T < MAXT) reveal_turn(state, T);
    output(hist);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)replay_true_score(hist));
    fprintf(stderr, "rounds = %lld, rollouts = %lld, scenarios/turn = %d..%d, time = %.1f ms\n",
            g_rounds, g_rollouts, g_min_sc == INT32_MAX ? 0 : g_min_sc, g_max_sc, (double)timer.ms());
    // ★実装のバグ検出: 進行中に積み上げたスコアと、出力を再生した真のスコアを比べる
    //   ※ 最終ターンの実現値は「次の reveal_turn()」で反映される作りなので、running 側は
    //      最後の 1 ターンぶんだけ足りない値になる。これは仕様であってバグではない。
    //      提出スコアは replay 側 (上の Score = ...) を見ること。それ以上にずれていたら
    //      calc_score / apply_move / reveal_turn のどれかが間違っている
    fprintf(stderr, "[check] running = %.0f / replay = %.0f\n",
            (double)state.score, (double)replay_true_score(hist));
    return 0;
}
