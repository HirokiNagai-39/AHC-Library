// =============================================================================
//  [焼きなまし系 09] センサー配置による被覆最大化 (max coverage)
// =============================================================================
//  【問題】
//    H×W のグリッドがあり、セル (i,j) には重み w_ij >= 0 がある。
//    センサーを K 個、好きなセルに置ける。センサーはチェビシェフ距離 R 以内のセルをすべて監視する。
//    1 つ以上のセンサーに監視されたセルの重みの合計を最大化せよ (重複は 1 回だけ数える)。
//  【入力】
//    H W K R
//    w_00 ... w_0(W-1)   (H 行)
//  【出力】
//    K 行。置いたセンサーの座標 "i j"
//  【スコア】 監視されたセルの重みの合計 (大きいほど良い)。
//  【入力生成方法】
//    H=W=40, K=15, R=4 固定。まず全セル w=0 とし、ランダムな中心 12 個について
//    中心から距離 d のセルに max(0, 9 - d) を加算する (クラスタ状の重み分布)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     最大被覆問題。貪欲 (いちばん新規被覆が大きいセルに 1 個ずつ置く) が定番だが、
//     貪欲は「後から見ると重なりすぎ」な配置になりやすい。
//     センサーを 1 個ずつ動かせる焼きなましなら、その重なりをほぐせる。
//     ★カテゴリ横断検証: 貪欲は 26183、焼きなましは 26991 で焼きなましが 3.1% 良かったため、
//       貪欲系ではなくこちらに置いている。
//
//   ● 状態 (State)
//     K 個のセンサーの位置 pos[] と、
//     cnt[セル] = 「そのセルを覆っているセンサーの個数」。
//     cnt があると「0 になったセルは重みを失う / 0 から 1 になったセルは重みを得る」で差分が取れる。
//
//   ● 手の作り方
//     センサーを 1 個選び、確率 P_NB で「半径 R の範囲内へ動かす」(微調整)、
//     残りでランダムなセルへ飛ばす。
//
//   ● 評価値
//     1 個以上のセンサーに覆われたセルの重みの合計。最大化 (重複は 1 回だけ数える)。
//
//   ● 差分計算 / 高速化
//     「いったん今の位置の被覆を外す → 新しい位置に置いてみる」で差分が出る。O(R^2) = 81。
//     ★ここが要注意ポイント: 状態を元に戻すときの増減まで同じ変数に足してしまうと、
//     差分が常に 0 になる (行って戻って合計 0)。
//     差分の値は最初の 2 手で確定させ、戻すときは別の変数に捨てること。
//     実際にこのバグを踏んで「スコアが初期値のまま動かない」現象になった。
//
//   ● つまずきポイント
//     ・上記の「戻す操作の増減を数えてしまう」バグ。スコアが動かないのに見た目は正常なので気付きにくい。
//     ・cnt[] を State に持たずグローバルにすると、ライブラリの best 解コピーと整合しなくなる。
//
//   ● さらに伸ばすなら
//     ・既に上限に到達している。制限時間を 15 秒 (8 倍) にしても seed 1/2 のスコアは動かず、
//       「センサー 1 個を全 1600 マスの中で最良の位置へ置き直す」強力な近傍
//       (未被覆重みの二次元累積和で O(HW)) を作っても seed 0-9 の 10 個すべてで 1 点も動かなかった。
//     ・伸ばすなら K や R を大きくして問題自体を難しくする方向
//
//  【採用したライブラリ】 simulated annealing/2_hill_climbing_kick
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=26495 / 2_hill_climbing_kick=27010 / 3_simulated_annealing=26996 / 4_multi_start_annealing=27005 / 5_iterated_annealing=27010
// =============================================================================
// =============================================================================
//  キックあり山登り (Iterated Local Search)   ---  AHC 用 高速テンプレート
// =============================================================================
//  山登りが局所解で止まったら「キック」で解を大きく動かし、最良解から探索し直す。
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct State { float score; ... };  ... 状態 (float score は必須)
//    init_state(State&)   ... 初期解構築 (state.score も必ずセットする)
//    modify(State&)       ... 遷移(近傍)を 1 つ選ぶ ★この時点では state を変えない
//    calc_score(State&)   ... 選んだ遷移を適用したときの「スコア差分」を返す
//    apply_move(State&)   ... 採用が決まった遷移を実際に state へ反映する
//
//  【なぜ「選ぶ → 差分計算 → 採用時だけ適用」なのか】
//    山登り/焼きなましでは提案の大半が却下される。却下時に何もしないこの順序が最速。
//    「先に適用して却下時に戻す」方式にしたい場合は
//      ・modify() の中で適用まで行う
//      ・apply_move() を空にする
//      ・ループ中の "// else rollback(state);" のコメントを外し rollback() を書く
//
//  【高速化のポイント】
//    ・乱数は xorshift128 (剰余を使わない [0,n) 生成)
//    ・時間計測は rdtsc(x86) / cntvct(ARM) を直接読む (chrono の ~1/10 のコスト)
//    ・時間計測は ITER_PER_CHECK 回に 1 回だけ (内側ループには時間計測が一切無い)
//    ・exp() を使わず log(乱数) のテーブル比較で採用判定 (焼きなまし系)
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
    // 32bit 乱数
    inline uint32_t next() {
        uint32_t t = x ^ (x << 11);
        x = y; y = z; z = w;
        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    }
    // [0, n) の整数 (剰余 % を使わないので高速)
    inline uint32_t next(uint32_t n) { return (uint32_t)(((uint64_t)next() * n) >> 32); }
    // [l, r) の整数
    inline uint32_t next(uint32_t l, uint32_t r) { return l + next(r - l); }
    // [0, 1) の float
    inline float nextf() { return (float)(next() >> 8) * (1.0f / 16777216.0f); }
    // [0, 1) の double
    inline double nextd() { return (double)next() * (1.0 / 4294967296.0); }
};
static Xor128 rng;

// xorshift による高速シャッフル
template <class T> inline void rnd_shuffle(T *a, int n) {
    for (int i = n - 1; i > 0; i--) swap(a[i], a[rng.next(i + 1)]);
}

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
        ms_per_tick_ = 1e-6; calib_ = 100;                 // chrono(ns) はそのまま
#endif
        w0_ = chrono::steady_clock::now();
        t0_ = tick();
    }
    // 経過時間[ms] (ホットループから呼ぶのはこれ)
    inline float ms() {
        uint64_t d = tick() - t0_;
        if (calib_ < 4) calibrate(d);       // 最初の数回だけ実時計で周波数を補正 (誤差 <0.01%)
        return (float)((double)d * ms_per_tick_);
    }
    void calibrate(uint64_t d) {
        double w = chrono::duration<double, milli>(chrono::steady_clock::now() - w0_).count();
        if (w > 0.25 * (double)(1u << (2 * calib_))) {      // 0.25ms, 1ms, 4ms, 16ms 時点で補正
            ms_per_tick_ = w / (double)d;
            calib_++;
        }
    }
};
static Timer timer;

// =============================================================================
// 3. パラメータ (optuna から環境変数 p1, p2, ... で上書きできる)
// =============================================================================
// optimize.py の generate_params() のキーと対応させる。
// 環境変数が無ければ既定値が使われるので、そのまま提出して良い。
template <class T> inline void pick_env(const char *key, T &dst) {
    if (const char *s = getenv(key)) {
        char *e = nullptr;
        double v = strtod(s, &e);
        if (e != s) dst = (T)v;
    }
}

// ---- 調整パラメータ (p1, p2, ... が optimize.py のキーと対応) ----
float P_NB             = 0.5f;  // p1: 近くへ動かす確率
int   KICK_STRENGTH    = 3;      // p2: キック 1 回で無条件に適用するランダム遷移の回数
int   STAGNATION_LIMIT = 10000;  // p3: 改善が無い回数がこれを超えたらキックする

void load_params() {
    pick_env("p1", P_NB);
    pick_env("p2", KICK_STRENGTH);
    pick_env("p3", STAGNATION_LIMIT);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化
// constexpr bool MAXIMIZE = false;    // ← スコア最小化

// gain (改善量) = GAIN_SIGN * スコア差分。 gain > 0 なら「良くなった」
constexpr float GAIN_SIGN = MAXIMIZE ? 1.0f : -1.0f;
// a が b より良ければ true
inline bool is_better(float a, float b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS   = 1900.0f;  // 全体の時間制限[ms] (実行時間制限 - 余裕)
constexpr int   ITER_PER_CHECK  = 1024;     // ★ 時間計測はこの回数に 1 回だけ行う
constexpr bool  ADAPTIVE_BLOCK  = true;     // 1 ブロックの実行時間が一定になるよう回数を自動調整
constexpr float TARGET_BLOCK_MS = 0.5f;     // ADAPTIVE_BLOCK=true のときの 1 ブロックの目標時間[ms]

// ---- 山登り用 ----
constexpr bool  ACCEPT_EQUAL = true;    // 同スコア(横ばい)の遷移も採用する → 平坦な地形を抜けやすい
constexpr float EQ_EPS       = 1e-6f;   // 「同スコア」とみなす誤差 (スコアの大きさに応じて調整)
constexpr float ACCEPT_TH    = ACCEPT_EQUAL ? -EQ_EPS : 0.0f;   // gain > ACCEPT_TH なら採用

// 1 ブロック(= 時間計測 1 回分)の反復回数を調整する。
// 近傍が重い問題でも時間超過せず、軽い問題では時間計測の回数を減らせる。
inline void tune_block(int &block, float dt) {
    if constexpr (ADAPTIVE_BLOCK) {
        if (dt > 1e-4f) {
            const double nb = (double)block * ((double)TARGET_BLOCK_MS / (double)dt);
            block = (int)min(max(nb, 32.0), 1.0e7);
        }
    }
}

// 統計 (デバッグ用。不要なら消して良い)
static ll g_iter = 0, g_accept = 0;
static ll g_kick = 0;

// #############################################################################
// # 6. ■ 問題ごとに書き換える部分 ■  (センサー配置 / 焼きなまし版)
// #############################################################################
constexpr int MAXH = 48, MAXW = 48, MAXC = MAXH * MAXW, MAXK = 64;
int H, W, KS, R;
static int WT[MAXC];

struct State { float score; int16_t pos[MAXK]; int16_t cnt[MAXC]; };
struct Move { int idx, to; };
static Move g_mv;

static inline void cov_add(State &s, int c, int dv, float &gain) {
    const int ci = c / W, cj = c % W;
    for (int i = max(0, ci - R); i <= min(H - 1, ci + R); i++)
        for (int j = max(0, cj - R); j <= min(W - 1, cj + R); j++) {
            const int d = i * W + j;
            if (dv > 0) { if (s.cnt[d]++ == 0) gain += (float)WT[d]; }
            else        { if (--s.cnt[d] == 0) gain -= (float)WT[d]; }
        }
}
float full_score(const State &s) {
    float t = 0.0f;
    for (int c = 0; c < H * W; c++) if (s.cnt[c] > 0) t += (float)WT[c];
    return t;
}
void init_state(State &s) {
    memset(s.cnt, 0, sizeof(int16_t) * (size_t)(H * W));
    s.score = 0.0f;
    for (int k = 0; k < KS; k++) { s.pos[k] = (int16_t)rng.next((uint32_t)(H * W)); float g = 0; cov_add(s, s.pos[k], 1, g); s.score += g; }
}
inline void modify(State &s) {
    g_mv.idx = (int)rng.next((uint32_t)KS);
    if (rng.nextf() < P_NB) {                     // 近くへ動かす
        const int p = s.pos[g_mv.idx], pi = p / W, pj = p % W;
        const int ni = min(H - 1, max(0, pi + (int)rng.next(2 * R + 1) - R));
        const int nj = min(W - 1, max(0, pj + (int)rng.next(2 * R + 1) - R));
        g_mv.to = ni * W + nj;
    } else g_mv.to = (int)rng.next((uint32_t)(H * W));
}
inline float calc_score(State &s) {
    if (s.pos[g_mv.idx] == g_mv.to) return 0.0f;
    float d = 0.0f, dummy = 0.0f;
    cov_add(s, s.pos[g_mv.idx], -1, d);           // いったん外して (減った分が d に入る)
    cov_add(s, g_mv.to, 1, d);                    // 新しい位置に置いてみる (増えた分が d に入る)
    cov_add(s, g_mv.to, -1, dummy);               // 状態だけ元に戻す (差分は d に確定済み)
    cov_add(s, s.pos[g_mv.idx], 1, dummy);
    return d;
}
inline void apply_move(State &s) {
    float d = 0.0f;
    cov_add(s, s.pos[g_mv.idx], -1, d);
    cov_add(s, g_mv.to, 1, d);
    s.pos[g_mv.idx] = (int16_t)g_mv.to;
}
void read_input() {
    if (scanf("%d %d %d %d", &H, &W, &KS, &R) == 4 && H >= 1) {
        H = min(H, MAXH); W = min(W, MAXW); KS = min(KS, MAXK);
        for (int i = 0; i < H * W; i++) scanf("%d", &WT[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 33);
        H = 40; W = 40; KS = 15; R = 4;
        memset(WT, 0, sizeof(int) * (size_t)(H * W));
        for (int t = 0; t < 12; t++) {
            const int ci = (int)g.next((uint32_t)H), cj = (int)g.next((uint32_t)W);
            for (int i = 0; i < H; i++) for (int j = 0; j < W; j++) {
                const int d = max(abs(i - ci), abs(j - cj));
                if (d < 9) WT[i * W + j] += 9 - d;
            }
        }
    }
}
void output(const State &s) { for (int k = 0; k < KS; k++) printf("%d %d\n", (int)s.pos[k] / W, (int)s.pos[k] % W); }
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
// 7. キックあり山登り (Iterated Local Search) 本体
// =============================================================================
//  山登りで局所解に落ちたら「キック」で解を強制的に大きく動かし、また山登りする。
//  キック前に必ず最良解へ戻すので、解が悪くなりっぱなしになることはない。
//  焼きなましの温度調整が難しい問題でも、パラメータが直感的で扱いやすい。

// キック: 近傍遷移を KICK_STRENGTH 回、採用判定なしで適用して解を大きく動かす
// (専用のキック近傍 ―― TSP なら double bridge など ―― を書くとより強力)
inline void kick(State &s) {
    for (int t = 0; t < KICK_STRENGTH; t++) {
        modify(s);
        const float d = calc_score(s);
        apply_move(s);
        s.score += d;                   // 差分を足していけばスコアの再計算は不要
    }
}

void hill_climbing_kick(State &s, float deadline_ms) {
    static State best;                  // 最良解 (キック前に必ずここへ戻る)
    best = s;
    int   stag  = 0;                    // 改善が無かった連続回数
    int   block = ITER_PER_CHECK;       // ★ 時間計測はこの回数に 1 回だけ
    float prev  = timer.ms();
    while (true) {
        // ---- 内側ループ: 時間計測を一切しない ----
        for (int it = 0; it < block; it++) {
            modify(s);
            const float diff = calc_score(s);
            const float gain = GAIN_SIGN * diff;
            if (gain > ACCEPT_TH) {
                apply_move(s);
                s.score += diff;
                g_accept++;
                if (gain > 0.0f) {              // 厳密に改善した = まだ登れているのでキックしない
                    stag = 0;
                    if (is_better(s.score, best.score)) best = s;   // 最良解を更新
                    continue;
                }
            }
            // else rollback(s);   // ←「先に適用して却下時に戻す」方式にする場合はここ
            // STAGNATION_LIMIT 回連続で一度も改善しなかった = 局所解に到達したとみなす
            if (++stag >= STAGNATION_LIMIT) {
                s = best;                       // 最良解に戻してから
                kick(s);                        // ランダムに大きく動かす
                stag = 0;
                g_kick++;
            }
        }
        g_iter += block;

        // ---- ブロックごとの処理 (時間計測はここだけ) ----
        const float now = timer.ms();
        if (now >= deadline_ms) break;
        tune_block(block, now - prev); prev = now;
    }
    if (is_better(best.score, s.score)) s = best;
}

// =============================================================================
//  ★差分計算の自動検証 (main の最初に走る)
// =============================================================================
//  下の [check] は「最後に積み上げたスコア」と「全計算したスコア」を比べるだけなので、
//    ・却下された手の calc_score が間違っている (焼きなましでは大半の手が却下される)
//    ・apply_move が s.score を上書きしていて差分の誤りが打ち消されている
//    ・誤差が偶然打ち消し合っている
//  といったバグを見逃す。これらは「探索が弱くなるだけで最後の値は合う」ので特に気付きにくい。
//
//  そこでここでは 1 手ずつ
//     「calc_score() の返り値」 対 「実際に apply_move して full_score() を取り直した変化量」
//  を突き合わせる。採用・却下に関係なく全部の手を検査するので、上の 3 つとも捕まえられる。
//
//  ※ 検証中は「必ず手を適用して先へ進む」。State の外 (グローバル変数) に差分計算用の情報を
//     持つ実装だと、状態だけ巻き戻しても整合が取れず誤検出になるため。
//     ランダムウォークになるので、初期解の周りだけでなく色々な状態を通る。
//  ※ 乱数の状態は前後で復元するので、VERIFY_MOVES を変えても探索結果は 1 ビットも変わらない。
//  ※ 提出時に消したければ VERIFY_MOVES = 0 にする (数 ms しか掛からないので普段は付けたままで良い)。
constexpr int VERIFY_MOVES = 200;

static void verify_diff() {
    if constexpr (VERIFY_MOVES <= 0) return;
    const Xor128 save = rng;                       // 乱数列を汚さないよう退避
    static State s;
    init_state(s);
    int bad = 0;
    for (int i = 0; i < VERIFY_MOVES; i++) {
        const float before = full_score(s);
        modify(s);
        const float diff = calc_score(s);
        apply_move(s);                             // ★必ず適用して先へ進む (下の注意を参照)
        s.score += diff;                           // フレームワークと同じ更新をする
        const float after = full_score(s);
        const float act = after - before;          // 実際に起きたスコアの変化
        // 許容誤差は「全計算の float 丸め (合計値に比例)」+「差分そのものの 0.1%」。
        // 合計値だけを基準にすると、合計が大きい問題で差分の誤りが埋もれてしまう。
        const float tol = 1e-4f * max(1.0f, max(fabsf(before), fabsf(after)))
                        + 1e-3f * max(fabsf(diff), fabsf(act));
        if (fabsf(act - diff) > tol) {
            if (++bad <= 5)
                fprintf(stderr, "[verify] NG %d 手目: calc_score=%.4f / 実際の変化=%.4f (ずれ %.4f)\n",
                        i, (double)diff, (double)act, (double)(act - diff));
        }
    }
    if (bad) fprintf(stderr, "[verify] ★差分計算が %d/%d 手でずれています。calc_score か apply_move にバグがあります\n", bad, VERIFY_MOVES);
    else     fprintf(stderr, "[verify] 差分計算 OK (%d 手を全計算と突き合わせ)\n", VERIFY_MOVES);
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
    verify_diff();      // ★差分計算の自動検証 (提出時に不要なら VERIFY_MOVES = 0)

    static State state; // State が大きくても良いように static 領域へ
    init_state(state);  // ★ 初期解構築
    hill_climbing_kick(state, TIME_LIMIT_MS);

    output(state);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)state.score);
    fprintf(stderr, "iter = %lld, accept = %lld (%.2f%%), time = %.1f ms\n",
            g_iter, g_accept, 100.0 * (double)g_accept / (double)max(1LL, g_iter), (double)timer.ms());
    fprintf(stderr, "kick = %lld\n", g_kick);
    // ★差分計算のバグ検出: 下の 2 つがずれていたら calc_score / apply_move が間違っている
    fprintf(stderr, "[check] diff-sum = %.3f, full = %.3f\n",
            (double)state.score, (double)full_score(state));
    return 0;
}
