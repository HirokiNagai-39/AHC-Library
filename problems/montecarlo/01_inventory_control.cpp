// =============================================================================
//  [モンテカルロ系 01] 在庫管理 (需要が確率的な発注計画)
// =============================================================================
//  【問題】
//    T ターンにわたって 1 種類の商品の在庫を管理する。初期在庫は I0。
//    毎ターンの流れは次の通り。
//      1. L ターン前に発注した商品が入荷する
//      2. 発注量 q (0..QMAX の 10 刻み) を決める。1 個あたり仕入値 PC。q は L ターン後に入荷する
//      3. その日の需要 d が実現する (事前には分からない。分布だけ分かっている)
//         在庫から出荷し、足りない分 1 個につき欠品ペナルティ SP
//      4. 残った在庫 1 個につき保管費 HC
//    T ターンの総コスト (仕入 + 欠品 + 保管) を最小化せよ。
//  【入力】
//    T L I0 QMAX PC SP HC DMEAN DVAR
//    d_0 ... d_{T-1}    (ジャッジ側の需要。解答側は turn に到達するまで見てはいけない)
//  【出力】
//    T 行。そのターンの発注量。
//  【スコア】 総コスト (小さいほど良い)。
//  【入力生成方法】
//    T=120, L=3, I0=100, QMAX=200, PC=5, SP=40, HC=2 固定。
//    需要 d_t は「平均 DMEAN=50、幅 DVAR=40 の一様分布」つまり [30,70] の一様整数。
//    ただし t が 30..50 と 80..95 の期間は需要が 1.8 倍になる (季節変動)。
//    ※ 解答側は「一様分布 [30,70]」だけを知っている。季節変動は知らない設定 (モデル誤差がある)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     発注した時点では需要が分からないので、「この発注量にしたら最終的にいくら掛かるか」を
//     未来の需要をランダムに 1 本引いて (シナリオ) 最後までシミュレートし、
//     平均コストが最小の発注量を選ぶ。
//     ★全候補が**同じシナリオ集合**を試すので、運の良い需要を引いた候補が勝つことがない (共通乱数法)。
//     この問題でいちばん効いたのは探索の工夫ではなく**モデルを直すこと**だった。
//     「需要は一様分布 [30,70]」という前提は現実 (季節変動で 1.8 倍になる期間がある) とずれている。
//     シナリオをその誤った前提で作っている限り、シナリオを何万本回しても正しい判断にはならない。
//
//   ● 状態 (State)
//     在庫、リードタイムぶんの入荷予定 (パイプライン)、ターン、そして推定した需要水準。
//
//   ● 手の作り方
//     発注量 0, 10, 20, ..., QMAX の離散候補。
//
//   ● 評価値 / rollout の方策
//     rollout は base-stock 方策:
//       「在庫 + 入荷予定」が  ROLLOUT_TH × 予測需要 × (リードタイム + 1)  を下回ったら補充する。
//     rollout の中でもシナリオの需要で予測を更新するので、
//     「上振れしたら発注も増える」という現実の運用と同じ挙動になる。
//     期末に届かない発注はしない / 残り需要より多くは積まない、という打ち切りも入れてある。
//
//   ● 需要水準の推定 (ここが本体)
//     観測済みの需要だけから水準を推定し直す。
//     推定は**非対称 EWMA**で、「上振れは即座に、下振れはゆっくり」追従させる (ALPHA_UP / ALPHA_DN)。
//     欠品ペナルティ 40 に対して保管費 2 と 20 倍の開きがあるので、多めに構える側へ倒すのが得。
//     さらにシナリオには「水準そのものの不確かさ」(SCEN_SIG) を重ねて、モデル誤差ぶんを織り込む。
//
//   ● つまずきポイント
//     ・rollout を速くしてシナリオ数を増やしても意味が無かった (MAX_SCENARIO=150 に絞っても
//       スコアはむしろわずかに良い)。シナリオ数は既に飽和しており、効くのはモデルの精度だけだった。
//     ・「窓内の上位 2 件をブレンドする予測」はオフラインでは EWMA より 3% 良かったのに、
//       モンテカルロに載せると +5% 悪化した。単体の予測精度と、方策に載せたときの良さは別物。
//
//   ● さらに伸ばすなら
//     ・需要のレジーム (平常期 / 繁忙期) を隠れマルコフモデルで推定する
//     ・(在庫, 予測水準) を状態にした動的計画法で最適発注量を直接求める
//
//  【改善】
//    (1) 需要水準を観測から推定する非対称 EWMA を入れ、シナリオ生成と rollout の base-stock 目標を
//        「固定の DMEAN」からその推定値に切り替えた (季節変動に数ターンで追従できるようになった)。
//    (2) シナリオに「水準そのものの不確かさ」(SCEN_SIG) を重ね、モデル誤差ぶんを織り込むようにした。
//    (3) 期末に届かない発注をしない / 残り需要を超えて積まない打ち切りを rollout の方策に追加。
//    seed 0..4 合計: 変更前 407260 -> 変更後 323692 (-20.5%)
//    seed 5..9 合計: 変更前 416528 -> 変更後 324766 (-22.0%)   ※小さいほど良い
//
//  【採用したライブラリ】 monte carlo/1_monte_carlo
//    実測比較 (seed 0,1,2 の合計スコア): 1_monte_carlo=250270 / 2_successive_halving=252878
// =============================================================================
// =============================================================================
//  モンテカルロ法 (Monte Carlo)   ---  AHC 用 高速テンプレート
// =============================================================================
//  各候補手について同じシナリオ集合を試し、期待値が最良の手を選ぶ。全候補が必ず同数のシナリオを試す完全公平版。
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
float ROLLOUT_TH = 1.25f;  // p1: base-stock 方策の目標在庫係数 (予測需要 x (L+1) に掛ける)
float TIME_ALPHA = 0.0f;   // p2: ターンごとの時間配分 (0=均等, 大きいほど序盤に厚く)
float ALPHA_UP   = 1.00f;  // p3: 需要 EWMA の学習率 (上振れ時。1 なら直ちに追従する)
float ALPHA_DN   = 0.15f;  // p4: 需要 EWMA の学習率 (下振れ時。ゆっくり戻す)
float FC_W0      = 0.375f; // p5: 予測を「既知の平均 DMEAN」へ引き戻す重み
float SCEN_SIG   = 0.15f;  // p6: シナリオごとの需要水準の不確かさ (モデル誤差ぶん)
float ROLLOUT_B  = -20.0f; // p7: base-stock 目標の定数オフセット
int   MAX_SCENARIO = 1 << 30;  // 1 ターンで回すシナリオ数の上限 (MAX_SCENARIO=1 で貪欲相当)

void load_params() {
    pick_env("p1", ROLLOUT_TH);
    pick_env("p2", TIME_ALPHA);
    pick_env("p3", ALPHA_UP);
    pick_env("p4", ALPHA_DN);
    pick_env("p5", FC_W0);
    pick_env("p6", SCEN_SIG);
    pick_env("p7", ROLLOUT_B);
    pick_env("MAX_SCENARIO", MAX_SCENARIO);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
// constexpr bool MAXIMIZE = true;     // ← スコア最大化
constexpr bool MAXIMIZE = false;       // ← スコア最小化

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
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (在庫管理)
// #############################################################################
constexpr int MAXT = 400, MAXL = 8;
constexpr int MAX_CAND = 32;

int T, LEAD, I0, QMAX, PC, SP, HC, DMEAN, DVAR;
static int TRUE_D[MAXT];              // ★ジャッジ側の需要 (turn に到達するまで見てはいけない)

struct Move { int16_t q; };

static float g_fc = 0.0f;             // 観測(過去)だけから作った 1 期あたりの需要予測

struct State {
    float   score;                    // 累積コスト
    float   ew;                       // 需要の EWMA (rollout の中でも更新する)
    int32_t inv;                      // 在庫
    int32_t pipe[MAXL];               // pipe[k] = k ターン後に入荷する量
    int32_t turn;
};

// 観測した需要から予測を更新する (上振れは速く、下振れはゆっくり追従させる)
static inline void fc_update(float &ew, int d) {
    const float a = ((float)d > ew) ? ALPHA_UP : ALPHA_DN;
    ew += a * ((float)d - ew);
}
static inline float fc_of(float ew) { return FC_W0 * (float)DMEAN + (1.0f - FC_W0) * ew; }

// ★必須: turn になって初めて分かる情報を state に入れる
//   (前のターンの需要が実現し、出荷・欠品・保管費が確定する。その需要で予測も更新する)
inline void reveal_turn(State &s, int turn) {
    if (turn > 0) {
        const int d = TRUE_D[turn - 1];
        const int ship = min(s.inv, d);
        s.inv -= ship;
        s.score += (float)SP * (float)(d - ship);     // 欠品
        s.score += (float)HC * (float)s.inv;          // 保管
        fc_update(s.ew, d);                           // ★過去の観測だけで予測を更新
    }
    // 入荷
    s.inv += s.pipe[0];
    for (int k = 0; k + 1 < LEAD; k++) s.pipe[k] = s.pipe[k + 1];
    s.pipe[LEAD - 1] = 0;
    s.turn = turn;
    g_fc = fc_of(s.ew);
}

void init_state(State &s) {
    s.score = 0.0f; s.inv = I0; s.turn = 0; s.ew = (float)DMEAN;
    for (int k = 0; k < LEAD; k++) s.pipe[k] = 0;
    g_fc = fc_of(s.ew);
}

inline int enum_moves(const State &s, Move *out) {
    (void)s;
    int m = 0;
    for (int q = 0; q <= QMAX && m < MAX_CAND; q += 10) out[m++].q = (int16_t)q;
    return m;
}
// 発注した時点で確定するのは仕入値だけ (需要はまだ分からない)
inline float calc_score(const State &s, const Move &mv) { (void)s; return (float)PC * (float)mv.q; }
inline void apply_move(State &s, const Move &mv) { s.pipe[LEAD - 1] += mv.q; }

// ★必須: シナリオ = 未来の需要列 (ターン番号で引ける配列にする)
//   予測水準 g_fc のまわりに「水準そのものの不確かさ (SCEN_SIG)」と
//   「1 期ごとのばらつき ([1-r, 1+r] 倍)」を重ねて引く。
struct Scenario { int16_t d[MAXT]; };
inline void gen_scenario(Scenario &sc, Xor128 &rnd, int from) {
    const float m    = 1.0f + SCEN_SIG * (rnd.nextf() * 2.0f - 1.0f);
    const float base = g_fc * m;
    const float lo   = base * (1.0f - (float)DVAR / (2.0f * (float)DMEAN));
    const float span = base * ((float)DVAR / (float)DMEAN);
    for (int t = max(0, from - 1); t < T; t++) {
        int d = (int)(lo + span * rnd.nextf() + 0.5f);
        sc.d[t] = (int16_t)(d > 0 ? d : 0);
    }
}

// ★必須: s の状態から最後までシミュレートして最終コストを返す
//   方策は「予測需要に追従する base-stock」。rollout の中でも予測を更新するので
//   シナリオが上振れすれば発注も増える (現実の運用と同じ挙動になる)。
inline float rollout(State &s, const Scenario &sc, Xor128 &rnd) {
    (void)rnd;
    float total = s.score;
    int inv = s.inv, pipe[MAXL];
    for (int k = 0; k < LEAD; k++) pipe[k] = s.pipe[k];
    float ew = s.ew;
    for (int t = s.turn; t < T; t++) {
        // 需要が実現
        const int d = sc.d[t];
        const int ship = min(inv, d);
        inv -= ship;
        total += (float)SP * (float)(d - ship);
        total += (float)HC * (float)inv;
        fc_update(ew, d);
        if (t + 1 >= T) break;
        // 入荷
        inv += pipe[0];
        for (int k = 0; k + 1 < LEAD; k++) pipe[k] = pipe[k + 1];
        pipe[LEAD - 1] = 0;
        // base-stock 方策: 在庫 + 入荷予定が目標を下回ったら補充する
        int q = 0;
        if (t + 1 + LEAD < T) {                       // 期末に届かない発注は無駄なのでしない
            const float f = fc_of(ew);
            float target = ROLLOUT_TH * f * (float)(LEAD + 1) + ROLLOUT_B;
            const float rem = f * (float)(T - (t + 1));   // 残り需要より多くは積まない
            if (target > rem) target = rem;
            int onhand = inv;
            for (int k = 0; k < LEAD; k++) onhand += pipe[k];
            q = (int)target - onhand;
            if (q < 0) q = 0; else if (q > QMAX) q = QMAX;
            q = (q / 10) * 10;
        }
        pipe[LEAD - 1] += q;
        total += (float)PC * (float)q;
    }
    return total;
}

void read_input() {
    if (scanf("%d %d %d %d %d %d %d %d %d", &T, &LEAD, &I0, &QMAX, &PC, &SP, &HC, &DMEAN, &DVAR) == 9 && T >= 1) {
        T = min(T, MAXT); LEAD = min(max(LEAD, 1), MAXL);
        for (int t = 0; t < T; t++) scanf("%d", &TRUE_D[t]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 401);
        T = 120; LEAD = 3; I0 = 100; QMAX = 200; PC = 5; SP = 40; HC = 2; DMEAN = 50; DVAR = 40;
        for (int t = 0; t < T; t++) {
            int d = DMEAN - DVAR / 2 + (int)g.next((uint32_t)(DVAR + 1));
            if ((t >= 30 && t < 50) || (t >= 80 && t < 95)) d = (int)(d * 1.8f);
            TRUE_D[t] = d;
        }
    }
}

void output(const vector<Move> &hist) {
    string r; r.reserve(hist.size() * 5);
    for (const Move &mv : hist) { r += to_string((int)mv.q); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const vector<Move> &hist) {
    float total = 0.0f;
    int inv = I0, pipe[MAXL];
    for (int k = 0; k < LEAD; k++) pipe[k] = 0;
    for (int t = 0; t < T && t < (int)hist.size(); t++) {
        inv += pipe[0];
        for (int k = 0; k + 1 < LEAD; k++) pipe[k] = pipe[k + 1];
        pipe[LEAD - 1] = 0;
        const int q = hist[t].q;
        if (q < 0 || q > QMAX) { fprintf(stderr, "[error] 発注量が範囲外\n"); return -1.0f; }
        pipe[LEAD - 1] += q;
        total += (float)PC * (float)q;
        const int d = TRUE_D[t];
        const int ship = min(inv, d);
        inv -= ship;
        total += (float)SP * (float)(d - ship) + (float)HC * (float)inv;
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
// 7. モンテカルロ法 本体
// =============================================================================
//  1 ラウンド = 「シナリオを 1 本作り、全候補でそれを試す」。
//  ラウンド単位で回すので、全候補のシナリオ数が常に等しくなる (＝完全に公平)。
Move choose_move(const State &state, float deadline_ms) {
    const int C = enum_moves(state, g_cand);
    if (C <= 1) { g_min_sc = 0; return g_cand[0]; }      // 選択の余地なし → シミュレート不要

    // 候補を適用した直後の状態を作り置き (rollout ごとに作り直さない)
    for (int c = 0; c < C; c++) {
        g_base[c] = state;
        g_base[c].score += calc_score(state, g_cand[c]);
        apply_move(g_base[c], g_cand[c]);
        g_sum[c] = 0.0;
    }

    int   n     = 0;                    // 回したシナリオ数
    float prev  = timer.ms();
    float round = 0.0f;                 // 直近 1 ラウンドの実測時間
    while (n < MAX_SCENARIO) {
        // ★打ち切りはラウンド境界だけ。途中で切ると候補ごとにシナリオ数が変わって不公平になる
        if (n > 0 && prev + round * ROUND_MARGIN > deadline_ms) break;

        // --- シナリオを 1 本作る (全候補で共有 = 共通乱数法) ---
        Xor128 rs; rs.seed(g_round_id++);
        gen_scenario(g_sc, rs, state.turn + 1);
        Xor128 rp; rp.seed(0x9E3779B9u ^ g_round_id);    // 方策用の乱数 (候補間で同じ種)

        // --- 全候補を同じシナリオで試す ---
        for (int c = 0; c < C; c++) {
            g_work = g_base[c];
            Xor128 r = rp;                               // ★候補ごとに同じ状態から始める
            g_sum[c] += (double)rollout(g_work, g_sc, r);
        }
        n++;
        const float now = timer.ms();
        round = now - prev;
        prev  = now;
    }
    g_rounds += n; g_rollouts += (ll)n * C;
    g_min_sc = min(g_min_sc, n); g_max_sc = max(g_max_sc, n);

    // 全候補のシナリオ数が同じなので、平均を取らず合計のまま比較できる
    int best = 0;
    for (int c = 1; c < C; c++) if (is_better(g_sum[c], g_sum[best])) best = c;
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
        const Move mv = choose_move(state, g_deadline[t]);
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
