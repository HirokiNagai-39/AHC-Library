// =============================================================================
//  [貪欲系 07] 広告スケジューリング (放映枠の割当)
// =============================================================================
//  【問題】
//    T 個の放映枠 (時刻 0..T-1) があり、各枠にちょうど 1 本の広告を流す。
//    広告は C 種類。広告 c を時刻 t に流すと収益 rev[c][t] が得られるが、
//    同じ広告を連続して流すと飽きられ、連続 k 回目の収益は rev[c][t] / k になる。
//    また広告 c は全体で cap_c 回までしか流せない。総収益を最大化せよ。
//  【入力】
//    T C
//    cap_0 ... cap_{C-1}
//    rev[c][0] ... rev[c][T-1]   (C 行)
//  【出力】
//    T 行。時刻 t に流す広告番号。
//  【スコア】 総収益 (大きいほど良い)。cap 超過があれば 0 点。
//  【入力生成方法】
//    T=300, C=20 固定。広告 c にピーク時刻 peak_c を [0,T) 一様、幅 sig_c を [20,80] 一様で与え、
//    rev[c][t] = 1 + round(100 * exp(-(t-peak_c)^2 / (2*sig_c^2))) とする (山型の需要)。
//    cap_c = ceil(T / C * 1.5)。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「同じ広告を連続で流すと収益が 1/k に落ちる」「広告ごとに放映回数の上限がある」の 2 つが絡む。
//     目先の最大収益を選ぶだけの貪欲だと、人気広告の枠を序盤で使い切ってしまい、
//     後半の高収益な時間帯に流すものが無くなる。
//     そこで「その手を打った後を実際に貪欲で進めてみて、結果が良かった手を採る」
//     ローリングホライゾンを使う。先の枯渇が実際にシミュレーションに現れるので、温存を学べる。
//
//   ● 状態 (State)
//     現在のターン、直前に流した広告とその連続回数、広告ごとの放映済み回数。
//
//   ● 手の作り方
//     そのターンに流せる広告 (放映上限に達していないもの) が候補。
//     ただし全部試すとプレイアウトが重いので、その時刻の収益が上位 TOPK 本だけに絞る。
//     評価値は収益を超えないので、下位の広告が最良になることはほぼ無い。
//
//   ● 評価値
//     eval = 収益/k - GAMMA × 機会費用 + ALPHA × (残り放映可能回数の割合) × 収益/k
//     ・第 1 項: 連続 k 回目なら収益は 1/k になる、という実際の得点。
//     ・第 3 項: 残量が多い広告を優先して枠を使い切りやすくする補正 (既定 ALPHA=0 で無効)。
//     ・第 2 項が「機会費用 (影の価格)」。以下で詳しく説明する。
//
//     ── 機会費用 (opportunity cost / 影の価格) とは ──
//     広告 c は全体で cap_c 回しか流せない。つまり「放映権 1 回ぶん」は有限の資源で、
//     今 1 回使うと将来のどこかで 1 回使えなくなる。
//     このとき失うのは「将来いちばん良かった枠」ではない。1 回減っても残り q-1 回は使えるので、
//     失うのは **将来の上位 q 個のうち、いちばん下 (q 番目) の枠** だけである。
//
//       今使わない場合に将来取れる収益 : top1 + top2 + ... + top(q-1) + top(q)
//       今 1 回使った場合に将来取れる収益: top1 + top2 + ... + top(q-1)
//       差 (= 今 1 回使う代償)           :                              top(q)
//
//     ここで q = CAP[c] - cnt[c] は「今の 1 回を含めた残り放映可能回数」、
//     top(j) は「現在時刻より後の rev[c][*] を降順に並べた j 番目」。
//
//     ── 具体例 ──
//     広告 c の残り回数 q と、時刻 t より後の rev[c][*] の降順が 100, 80, 30, 10, 5 のとき
//       q = 1 (残り 1 回) → 代償 = top(1) = 100  … 一番おいしい枠を丸ごと失う。とても高い
//       q = 3 (残り 3 回) → 代償 = top(3) =  30  … 上位 2 つは残り 2 回でまだ取れるので、
//                                                  実際に失うのは 3 番目の 30 だけ
//       q = 5 以上        → 代償 = 5 以下        … 余っているのでほぼタダ
//     つまり「残りが少ない広告ほど高くつく」= ピーク前の広告が自然に温存され、
//     ピークを過ぎた (将来の枠が安い) 広告から先に消費される。
//
//     ── 前計算 (QV テーブル) ──
//     QV[t][c][q] = 「時刻 t より後で広告 c が取れる収益の q 番目に大きい値」を持つ。
//     作り方は、広告 c ごとに時刻を後ろ (t = T-1) から前へ走査し、
//     降順に並んだ上位 MAXQ 個の配列 top[] を挿入ソートで保ちながら、
//     各 t で「その時点の top[] のコピー」を QV[t][c][*] に書き出す (rev[c][t] を入れる前に書く)。
//     計算量は O(C × T × MAXQ)、メモリは int16_t で MAXT × MAXC × MAXQ = 約 2MB。
//     q = 0 は「もう使えない」なので 0 を入れてある (enum_moves で除外されるので実際には引かれない)。
//
//     ── なぜ GAMMA < 1 なのか ──
//     上の代償は「その広告が将来 top1..top(q) の枠を実際に取れる」という仮定に基づく緩和で、
//     楽観的すぎる。実際には
//       ・1 つの枠には 1 本しか流せない (他の広告と枠を奪い合う)
//       ・同じ広告を連続で流すと収益が 1/k に落ちる
//       ・他の広告にも容量があり、それらも同じ枠を狙っている
//     ので、本当の機会費用は QV より小さい。そのまま (GAMMA=1) 引くと請求しすぎになり、
//     「もったいなくて何も流せない」状態に近づく。実測でも GAMMA=1 は 0.2 より -0.08% 悪い。
//
//     ── 実測した効き ──
//     GAMMA=0 (機会費用なし) と GAMMA=0.2 の A/B は seed 0..9 で 279231 → 279351 (+0.04%、7/10 seed で改善)。
//     **効くことは効くが寄与はごく小さい**。この問題で +1.4% を作ったのは主に
//     「候補を収益上位 TOPK 本に絞って試行回数を約 6 倍にしたこと」と
//     「プレイアウトを 20 → 35 手に伸ばしたこと」の 2 つで、機会費用はその上乗せ。
//     考え方自体は資源制約付きの問題で広く使える (在庫・予算・回数制限などに同じ形で効く) ので、
//     テンプレートとして残してある。
//
//   ● 差分計算 / 高速化
//     収益は表引き、連続回数は State に持つので 1 手 O(1)。
//     重いのはプレイアウト (HORIZON 手 × 候補数) なので、候補を絞るのが効く。
//
//   ● つまずきポイント
//     ・候補を絞らないと 1 手のコストが候補数に比例して重くなり、プレイアウトが伸ばせない。
//     ・機会費用の重みは効きすぎると「温存しすぎて何も流せない」状態になる。必ず振って確かめること。
//
//   ● さらに伸ばすなら
//     ・容量制約付き割当 (連続ペナルティを無視した緩和) の上界が seed あたり約 29400 に対し
//       現状は 29000 前後なので、残りの伸びしろは 1〜2% 程度
//     ・連続ペナルティも含めた最小費用流での緩和を作れば、より正確な機会費用が出せる
//
//  【改善】
//    (1) 候補をその時刻の収益上位 3 本に絞り、試行回数を約 6 倍に増やした。
//    (2) 「残り容量を今使うと将来どれだけ損か」を表す機会費用 (影の価格) を評価値に入れた。
//        ※ 単独の効きは seed 0..9 で +0.04% と小さい (寄与の大半は (1) と (3))。
//    (3) プレイアウトの長さを 20 -> 35 手に伸ばした。
//    seed 0..4 合計: 変更前 136611 -> 変更後 138571 (+1.4%) ※seed 5..9 でも 138182 -> 140780 (+1.9%)
//
//  【採用したライブラリ】 greedy/3_rolling_horizon
//    実測比較 (seed 0,1,2 の合計スコア): 1_greedy=81482 / 3_rolling_horizon=85706
// =============================================================================
// =============================================================================
//  貪欲プレイアウト (ローリングホライゾン)   ---  AHC 用 高速テンプレート
// =============================================================================
//  各候補手について先を貪欲でプレイアウトし、結果が最良の手を採用する。1 と同じインターフェース。
//
//  【貪欲法の 3 つの型】
//    1_greedy            … 毎回「一番良い手」を選んで解の末尾に足していく (最速・最も単純)
//    2_insertion_greedy  … 末尾だけでなく「操作列の途中」にも挿入できる貪欲 (過去改変貪欲)
//    3_rolling_horizon   … 各候補手について先を貪欲でプレイアウトし、結果が最良の手を選ぶ
//
//  【ファイル構成】
//    1. 高速乱数 (xorshift128)      2. 高速タイマー (rdtsc / cntvct)
//    3. パラメータ (環境変数 = optuna)  4. 最大化 / 最小化 の切り替え
//    5. 高速化の設定                 6. ■ 問題ごとに書き換える部分
//    7. アルゴリズム本体             8. main
//
//  【書き換えるのは 6. だけ】
//    struct Move  { ... };                     ... 手
//    struct State { float score; ... };        ... 状態 (score は必須)
//    init_state(State&)                        ... 初期状態
//    enum_moves(const State&, Move*)           ... 今選べる手を全列挙して個数を返す (0 で終了)
//    eval_move(const State&, Move)             ... 貪欲の評価値 (大きいほど先に選ぶ)
//    calc_score(const State&, Move)            ... その手で確定するスコア差分
//    apply_move(State&, Move)                  ... 手を適用する (score は触らない)
//    ※ 3 は 1 と全く同じインターフェース。1 で書いたものをそのまま使える。
//
//  【高速化のポイント】
//    ・候補の評価は全ソートせず「最大のものを 1 パスで拾う」だけ (O(候補数))
//    ・State・候補配列はすべて静的確保。ループ中に malloc を一度も呼ばない
//    ・評価値にノイズを掛けたランダム貪欲を時間いっぱい繰り返し、最良解を採用する
//      (NOISE = 0 なら決定的な貪欲 1 回で終了する)
//    ・時間計測は rdtsc/cntvct 直読み。1 試行に 1 回だけ
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
float ALPHA     = 1.0f;    // p1: 残り容量の割合による補正の強さ
float EPS_DIST  = 1.0f;    // p2: 0 除算よけ
int   HORIZON   = 35;      // p3: プレイアウトで何手先まで進めるか (-1 なら最後まで)
float NOISE     = 0.05f;   // p4: プレイアウトのノイズ幅
float GAMMA     = 0.2f;    // p5: 機会費用 (将来捨てる収益) を引く強さ
int   TOPK      = 3;       // p6: 各時刻で見る候補数 (収益上位から。0 で全部)

void load_params() {
    pick_env("p1", ALPHA);
    pick_env("p2", EPS_DIST);
    pick_env("p3", HORIZON);
    pick_env("p4", NOISE);
    pick_env("p5", GAMMA);
    pick_env("p6", TOPK);
}

// =============================================================================
// 4. スコアの最大化 / 最小化 の切り替え  ★どちらか一方をコメントアウトする
// =============================================================================
constexpr bool MAXIMIZE = true;        // ← スコア最大化
// constexpr bool MAXIMIZE = false;    // ← スコア最小化

inline bool is_better(float a, float b) { if constexpr (MAXIMIZE) return a > b; else return a < b; }
constexpr float WORST_SCORE = MAXIMIZE ? -3.0e38f : 3.0e38f;

// =============================================================================
// 5. 高速化の設定
// =============================================================================
constexpr float TIME_LIMIT_MS = 1900.0f;   // 全体の時間制限[ms] (実行時間制限 - 余裕)
// (この手法は探索設定より「評価関数」と「プレイアウトの長さ」が効く)

// 統計 (デバッグ用。不要なら消して良い)
static ll g_trials = 0, g_steps = 0;

// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (広告スケジューリング)
// #############################################################################
constexpr int MAXT = 400, MAXC = 40;
constexpr int MAX_CAND = MAXC;             // ★候補は広告の種類数
constexpr int MAXQ = 64;                   // 機会費用テーブルで見る残り容量の上限

int T, C;
static int REV[MAXC][MAXT], CAP[MAXC];
static float INV[MAXT + 2];                // 1/k の前計算
static int8_t ORD[MAXT][MAXC];             // 各時刻で収益の高い順に並べた広告
// QV[t][c][q] = 時刻 t より後で広告 c が取れる収益の「q 番目に大きい値」
//   = 残り容量が q のときに「今 c を使うと捨てることになる将来の収益」(機会費用 / 影の価格)
static int16_t QV[MAXT][MAXC][MAXQ];

struct Move { int8_t ad; };

struct State {
    float   score;
    int32_t turn;
    int8_t  last;
    int16_t run;               // 直前の広告が何連続しているか
    int16_t cnt[MAXC];
    int8_t  plan[MAXT];
};

void init_state(State &s) {
    s.score = 0.0f; s.turn = 0; s.last = -1; s.run = 0;
    memset(s.cnt, 0, sizeof(int16_t) * (size_t)C);
}

// ★候補の絞り込み: その時刻で収益上位 TOPK 本だけを見る。
//   評価値は (収益 - 機会費用) ≦ 収益 なので、収益が低い広告が最良になることはほぼ無い。
inline int enum_moves(const State &s, Move *out) {
    if (s.turn >= T) return 0;
    int m = 0;
    if (TOPK > 0 && TOPK < C) {
        const int8_t *o = ORD[s.turn];
        for (int j = 0; j < C && m < TOPK; j++) { const int c = o[j]; if (s.cnt[c] < CAP[c]) out[m++].ad = (int8_t)c; }
        if (m == 0) for (int c = 0; c < C; c++) if (s.cnt[c] < CAP[c]) out[m++].ad = (int8_t)c;
    } else {
        for (int c = 0; c < C; c++) if (s.cnt[c] < CAP[c]) out[m++].ad = (int8_t)c;
    }
    return m;
}
inline float calc_score(const State &s, const Move &mv) {
    const int k = (mv.ad == s.last) ? (s.run + 1) : 1;
    return (float)REV[mv.ad][s.turn] * INV[k];
}
// 評価値: 今の収益 - GAMMA * 機会費用
//   「残り容量 q の広告を今使う」= 「将来 q 番目に良かった枠を捨てる」ので、その値を引く。
//   ピーク前の広告は機会費用が高くなって温存され、ピークを過ぎた広告から先に使われる。
//   ALPHA 項は従来どおり「残り容量の割合」で枠を使い切りやすくする補正 (既定 0)。
inline float eval_move(const State &s, const Move &mv) {
    const int k = (mv.ad == s.last) ? (s.run + 1) : 1;
    const float now = (float)REV[mv.ad][s.turn] * INV[k];
    int q = CAP[mv.ad] - s.cnt[mv.ad];
    if (q > MAXQ - 1) q = MAXQ - 1;
    const float lam = (float)QV[s.turn][mv.ad][q];
    const float rest = (float)(CAP[mv.ad] - s.cnt[mv.ad]) / (float)CAP[mv.ad];
    return now - GAMMA * lam + ALPHA * rest * now;
}
inline void apply_move(State &s, const Move &mv) {
    s.run = (mv.ad == s.last) ? (int16_t)(s.run + 1) : (int16_t)1;
    s.last = mv.ad;
    s.cnt[mv.ad]++;
    s.plan[s.turn] = mv.ad;
    s.turn++;
}

void read_input() {
    if (scanf("%d %d", &T, &C) == 2 && T >= 1) {
        T = min(T, MAXT); C = min(C, MAXC);
        for (int c = 0; c < C; c++) scanf("%d", &CAP[c]);
        for (int c = 0; c < C; c++) for (int t = 0; t < T; t++) scanf("%d", &REV[c][t]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 77);
        T = 300; C = 20;
        for (int c = 0; c < C; c++) {
            const float pk = (float)g.next((uint32_t)T);
            const float sg = 20.0f + g.nextf() * 60.0f;
            CAP[c] = (int)ceilf((float)T / (float)C * 1.5f);
            for (int t = 0; t < T; t++) {
                const float x = ((float)t - pk) / sg;
                REV[c][t] = 1 + (int)lroundf(100.0f * expf(-0.5f * x * x));
            }
        }
    }
    for (int k = 0; k <= T + 1; k++) INV[k] = (k <= 0) ? 1.0f : 1.0f / (float)k;
    // 各時刻の収益順
    for (int t = 0; t < T; t++) {
        int idx[MAXC];
        for (int c = 0; c < C; c++) idx[c] = c;
        sort(idx, idx + C, [t](int a, int b) { return REV[a][t] > REV[b][t]; });
        for (int c = 0; c < C; c++) ORD[t][c] = (int8_t)idx[c];
    }
    // 機会費用テーブル: 時刻を後ろから走査して「上位 MAXQ 個の収益」を保つ
    for (int c = 0; c < C; c++) {
        int top[MAXQ]; int n = 0;
        for (int t = T - 1; t >= 0; t--) {
            for (int q = 0; q < MAXQ; q++) QV[t][c][q] = (int16_t)((q == 0) ? 0 : (q - 1 < n ? top[q - 1] : 0));
            const int v = REV[c][t];                       // rev[c][t] を降順配列に挿入
            int p = n < MAXQ ? n : MAXQ - 1;
            while (p > 0 && top[p - 1] < v) { top[p] = top[p - 1]; p--; }
            top[p] = v;
            if (n < MAXQ) n++;
        }
    }
}

void output(const State &s) {
    string r; r.reserve((size_t)T * 3);
    for (int t = 0; t < s.turn; t++) { r += to_string((int)s.plan[t]); r += '\n'; }
    fputs(r.c_str(), stdout);
}

float replay_true_score(const State &s) {
    if (s.turn != T) { fprintf(stderr, "[error] 枠が埋まっていない\n"); return -1.0f; }
    static int cnt[MAXC]; memset(cnt, 0, sizeof(int) * (size_t)C);
    float total = 0.0f; int last = -1, run = 0;
    for (int t = 0; t < T; t++) {
        const int c = s.plan[t];
        if (c < 0 || c >= C) { fprintf(stderr, "[error] 不正な広告\n"); return -1.0f; }
        run = (c == last) ? run + 1 : 1; last = c;
        total += (float)REV[c][t] / (float)run;
        if (++cnt[c] > CAP[c]) { fprintf(stderr, "[error] 広告 %d が cap 超過\n", c); return -1.0f; }
    }
    return total;
}
// #############################################################################
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
// 7. 貪欲プレイアウト (ローリングホライゾン) 本体
// =============================================================================
//  1 手を決めるのに「その手を打った後を貪欲で HORIZON 手ぶん進めてみて、
//  結果が一番良かった手」を採用する。目先の評価値だけでは分からない
//  「この手を打つと後で詰む」を検出できるので、単純な貪欲よりかなり強い。
//  コストは (候補数 × プレイアウトの長さ) 倍になるので、時間配分に注意する。
static Move  g_cand[MAX_CAND];
static Move  g_cand2[MAX_CAND];
static State g_cur, g_best, g_work;

// s から貪欲で最大 h 手進めて、到達したスコアを返す (h < 0 なら最後まで)
static float playout(State &s, int h, float noise) {
    for (int step = 0; h < 0 || step < h; step++) {
        const int m = enum_moves(s, g_cand2);
        if (m == 0) break;
        int   bi = -1;
        float bv = WORST_SCORE;
        for (int k = 0; k < m; k++) {
            float e = eval_move(s, g_cand2[k]);
            if (e == WORST_SCORE) continue;
            if (noise > 0.0f) e *= 1.0f + noise * (rng.nextf() - 0.5f);
            if (is_better(e, bv)) { bv = e; bi = k; }
        }
        if (bi < 0) break;
        s.score += calc_score(s, g_cand2[bi]);
        apply_move(s, g_cand2[bi]);
        g_steps++;
    }
    return s.score;
}

// 1 回ぶん (最初から最後まで) を回す
static void run_rh(State &g_cur, float noise, float deadline_ms) {
    init_state(g_cur);
    while (true) {
        const int m = enum_moves(g_cur, g_cand);
        if (m == 0) break;
        if (m == 1) {                                   // 選択の余地なし
            if (eval_move(g_cur, g_cand[0]) == WORST_SCORE) break;
            g_cur.score += calc_score(g_cur, g_cand[0]);
            apply_move(g_cur, g_cand[0]);
            continue;
        }
        // ---- 各候補について「打った後を貪欲で進めた結果」を比べる ----
        int   bi = -1;
        float bv = WORST_SCORE;
        for (int k = 0; k < m; k++) {
            if (eval_move(g_cur, g_cand[k]) == WORST_SCORE) continue;   // 実行不可の手
            g_work = g_cur;                             // ★State のコピー
            g_work.score += calc_score(g_work, g_cand[k]);
            apply_move(g_work, g_cand[k]);
            const float v = playout(g_work, HORIZON, noise);
            if (is_better(v, bv)) { bv = v; bi = k; }
            if (timer.ms() > deadline_ms) break;        // 時間切れなら今までの中で最良を採用
        }
        if (bi < 0) break;
        g_cur.score += calc_score(g_cur, g_cand[bi]);
        apply_move(g_cur, g_cand[bi]);
    }
    g_trials++;
}

void rolling_horizon(float deadline_ms) {
    run_rh(g_best, 0.0f, deadline_ms);                 // まず決定的に 1 回
    if (NOISE <= 0.0f) return;
    while (timer.ms() < deadline_ms) {                 // 時間が余ったらノイズ付きで繰り返す
        run_rh(g_cur, NOISE, deadline_ms);
        if (is_better(g_cur.score, g_best.score)) g_best = g_cur;
    }
}

// =============================================================================
// 8. main
// =============================================================================
int main() {
    timer.start();      // ★ 一番最初にタイマー開始
    load_params();      // 環境変数からパラメータを読む (optuna 用)
    read_input();

    rolling_horizon(TIME_LIMIT_MS);
    output(g_best);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)replay_true_score(g_best));
    fprintf(stderr, "trials = %lld, steps = %lld, time = %.1f ms\n", g_trials, g_steps, (double)timer.ms());
    // ★実装のバグ検出: 積み上げたスコアと、出力を再生した真のスコアが一致するはず。
    //   貪欲は「選んだ手」でしか calc_score を呼ばないので、差分の誤りは必ずここに出る
    //   (焼きなましのように「却下された手の差分が間違っている」という隠れ方をしない)。
    //   なお eval_move は順位付け用のヒューリスティックなので、ここでは検査しない/する必要も無い。
    fprintf(stderr, "[check] running = %.0f / replay = %.0f\n",
            (double)g_best.score, (double)replay_true_score(g_best));
    return 0;
}
