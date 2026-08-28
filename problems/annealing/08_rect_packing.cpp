// =============================================================================
//  [焼きなまし系 08] 長方形の詰め込み (価値最大化)
// =============================================================================
//  【問題】
//    W×H のボードに、N 個の長方形を置く (置かなくても良い)。
//    長方形 i は幅 w_i、高さ h_i、価値 v_i を持ち、90 度回転して置いても良い。
//    座標は整数で、ボードからはみ出してはいけない。
//    スコア = (置いた長方形の価値の合計) - 100 * (長方形どうしが重なっている面積) を最大化せよ。
//    ※ 重なりが 0 の配置が目標だが、途中で重なりを許した方が探索しやすいのでこの形にしてある。
//  【入力】
//    W H N
//    w_i h_i v_i   (N 行)
//  【出力】
//    N 行。長方形 i を置くなら "x y rot" (左下座標と回転 0/1)、置かないなら "-1 -1 -1"。
//  【スコア】 上記の値 (大きいほど良い)。はみ出しがあれば 0 点。
//  【入力生成方法】
//    W=H=100, N=60 固定。w_i,h_i は 5..25 の一様整数、v_i = w_i*h_i * (0.8..1.2 の一様実数) を四捨五入。
//    面積の合計はボード面積の約 1.6 倍になるので、全部は入らない。
//    ※ 標準入力が無ければ上記の方法で内部生成する (環境変数 SEED で種を変えられる)。
//  【解法】
//   ● 考え方
//     「重ならないように置く」を厳密な制約にすると、近傍がほとんど動けなくなる
//     (少しずらすとすぐ他の長方形にぶつかる)。
//     そこで重なり面積にペナルティを掛けた評価値にして、自由に動かせるようにする (ペナルティ法)。
//     温度が高いうちは重なりを許して大きく動き、温度が下がるにつれて重なりが自然に解消されていく。
//     これが焼きなましの一番おいしい使い方。
//
//   ● 状態 (State)
//     各長方形の (置くかどうか on[], 左下座標 x[],y[], 回転 rot[])。
//     加えて高速化用に、左下と右上の座標を SoA (構造体の配列ではなく配列の構造体) で持つ。
//
//   ● 手の作り方
//     長方形を 1 つ選び、次の 5 種類を確率で使い分ける。
//       ・置く / 外すの切り替え (P_TOGGLE)
//       ・回転 (P_ROT)
//       ・盤面のどこかへ飛ばす (P_JUMP)
//       ・近くへ微移動 (SHIFT_W の範囲。盤外は端に丸めるので「壁に寄せる」効果もある)
//       ・他の長方形の辺にぴったり寄せる (スナップ、P_SNAP)
//     ランダム配置だけでは最後の詰めができないので、微移動とスナップが決め手になる。
//
//   ● 評価値
//     Σ (置いた長方形の価値) - 100 × (重なり面積の合計)。最大化。
//     この式そのものが問題のスコア定義なので、評価値と真のスコアが一致している。
//
//   ● 差分計算 / 高速化
//     動かす 1 個と他の全長方形の重なりだけを見れば良いので O(N)。
//     ここで「置いていない長方形を幅 0 の点として持つ」のが効く。
//     こうすると重なり判定から if 分岐が完全に消え、O(N) ループがそのまま SIMD 化される。
//     反復数が 9.5M から 55M へ 5.8 倍になった。
//
//   ● つまずきポイント
//     ・自分自身との重なりを数えないよう、計算中は自分の矩形を一時的に幅 0 にしておく。
//     ・微移動の幅は 20 -> 32 -> 50 と大きくするほど良くなり、70/90 では悪化した。
//       「盤外を端に丸める」効果と合わせて、この幅が壁寄せの強さを決めている。
//
//   ● さらに伸ばすなら
//     ・「重なっている相手の方向と逆へ押し出す」物理的な近傍
//     ・面積の大きい長方形から順に置く決定的な初期解 (現在は空から始めている)
//
//  【改善】
//    (1) 重なり計算を SoA + 分岐なしループにして SIMD 化 (反復数 9.5M -> 55M)。
//    (2) 近傍に「近くへ微移動」と「他の長方形の辺へスナップ」を追加 (従来は毎回ランダム配置だった)。
//    (3) 微移動幅 SHIFT_W / スナップ確率 P_SNAP を追加して調整 (SHIFT_W=50 が最良)。
//    seed 0..4 合計: 変更前 45404 -> 変更後 49768 (+9.6%)  (seed 0..9 でも全 seed 改善で +9.6%)
//
//  【採用したライブラリ】 simulated annealing/3_simulated_annealing
//    実測比較 (seed 0,1,2 の合計スコア): 1_hill_climbing=22412 / 2_hill_climbing_kick=22824 / 3_simulated_annealing=26764 / 4_multi_start_annealing=25636 / 5_iterated_annealing=26303
// =============================================================================
// =============================================================================
//  焼きなまし法 (Simulated Annealing)   ---  AHC 用 高速テンプレート
// =============================================================================
//  確率的に悪化も受け入れて局所解を脱出する。AHC で最もよく使われる手法。
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
float TEMP_START = 300.0f;   // p1: 開始温度
float TEMP_END   = 1.0f;     // p2: 終了温度
float P_TOGGLE   = 0.15f;    // p3: 置く/外すを切り替える確率
float P_ROT      = 0.10f;    // p4: (置いてある長方形に対し) 回転させる確率
float P_JUMP     = 0.25f;    // p5: 盤面のどこかへランダムに飛ばす確率 (残りは近傍への微移動)
int   SHIFT_W    = 50;       // p6: 微移動の幅 (盤の外へ出たら端に丸めるので、大きめだと壁に寄る効果もある)
float P_SNAP     = 0.30f;    // p7: 微移動のかわりに他の長方形の辺へぴったり寄せる確率

void load_params() {
    pick_env("p1", TEMP_START);
    pick_env("p2", TEMP_END);
    pick_env("p3", P_TOGGLE);
    pick_env("p4", P_ROT);
    pick_env("p5", P_JUMP);
    pick_env("p6", SHIFT_W);
    pick_env("p7", P_SNAP);
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

// ---- 焼きなまし用 ----
constexpr bool  KEEP_BEST      = true;   // 最良解を別に保持する (State のコピーが重いなら false)
constexpr float KEEP_BEST_FROM = 0.5f;   // 進捗率がこれを超えてから記録開始 (序盤の無駄なコピーを削減)

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


// #############################################################################
// # 6. ■ ここから 問題ごとに書き換える部分 ■  (長方形の詰め込み)
// #############################################################################
constexpr int MAXN = 128;
constexpr float OVER_PEN = 100.0f;

int BW, BH, N;
static int RW[MAXN], RH[MAXN], RV[MAXN];

struct State {
    float   score;
    int16_t x[MAXN], y[MAXN];
    uint8_t on[MAXN], rot[MAXN];
};

// 重なり計算を分岐なしのループにするための SoA (現在解 s と同期させて持つ)。
// 置いていない長方形は幅・高さ 0 の点にしておけば、判定なしで重なり 0 になる。
alignas(32) static int BX[MAXN], BY[MAXN], BX2[MAXN], BY2[MAXN];
static uint32_t g_soa_stamp = 0xffffffffu;
static const State *g_soa_src = nullptr;

struct Move { int i, nx, ny, non, nrot; };
static Move g_mv;

static inline int wof(int i, int r) { return r ? RH[i] : RW[i]; }
static inline int hof(int i, int r) { return r ? RW[i] : RH[i]; }

static inline void soa_set(const State &s, int i) {
    if (s.on[i]) {
        BX[i] = s.x[i]; BY[i] = s.y[i];
        BX2[i] = s.x[i] + wof(i, s.rot[i]); BY2[i] = s.y[i] + hof(i, s.rot[i]);
    } else { BX[i] = BY[i] = BX2[i] = BY2[i] = 0; }
}
static inline void soa_sync(const State &s) {
    for (int i = 0; i < N; i++) soa_set(s, i);
    for (int i = N; i < MAXN; i++) BX[i] = BY[i] = BX2[i] = BY2[i] = 0;
}

// 長方形 i を (x,y,w,h) に置いたときの、i 以外との重なり面積の合計。
// 分岐が無いので AVX2 で 8 個ずつ処理される (元の実装の数倍速い)。
static inline int overlap_sum(int x, int y, int w, int h) {
    const int x2 = x + w, y2 = y + h;
    int acc = 0;
    for (int j = 0; j < N; j++) {
        int ox = (x2 < BX2[j] ? x2 : BX2[j]) - (x > BX[j] ? x : BX[j]);
        int oy = (y2 < BY2[j] ? y2 : BY2[j]) - (y > BY[j] ? y : BY[j]);
        ox = ox > 0 ? ox : 0;
        oy = oy > 0 ? oy : 0;
        acc += ox * oy;
    }
    return acc;
}

float full_score(const State &s) {
    float t = 0.0f;
    for (int i = 0; i < N; i++) if (s.on[i]) {
        t += (float)RV[i];
        const int w = wof(i, s.rot[i]), h = hof(i, s.rot[i]);
        for (int j = i + 1; j < N; j++) {
            if (!s.on[j]) continue;
            const int w2 = wof(j, s.rot[j]), h2 = hof(j, s.rot[j]);
            const int ox = min((int)s.x[i] + w, (int)s.x[j] + w2) - max((int)s.x[i], (int)s.x[j]);
            if (ox <= 0) continue;
            const int oy = min((int)s.y[i] + h, (int)s.y[j] + h2) - max((int)s.y[i], (int)s.y[j]);
            if (oy <= 0) continue;
            t -= OVER_PEN * (float)(ox * oy);
        }
    }
    return t;
}

void init_state(State &s) {
    for (int i = 0; i < N; i++) { s.on[i] = 0; s.rot[i] = 0; s.x[i] = 0; s.y[i] = 0; }
    s.score = 0.0f;
    soa_sync(s); g_soa_src = &s;
}

inline void modify(State &s) {
    if (g_soa_src != &s) { soa_sync(s); g_soa_src = &s; }   // 解が差し替わっていたら張り直す
    const int i = (int)rng.next((uint32_t)N);
    g_mv.i = i;
    int non = (int)s.on[i], nrot = (int)s.rot[i], nx = (int)s.x[i], ny = (int)s.y[i];
    int mode;                                     // 0=そのまま 1=ランダム配置 2=微移動 3=寄せる
    if (rng.nextf() < P_TOGGLE) {                 // 置く / 外すを切り替える
        non = 1 - non;
        if (non) { nrot = (int)rng.next(2); mode = 1; } else mode = 0;
    } else if (!non) {                            // 置いていない → ランダムな位置に置く
        non = 1; nrot = (int)rng.next(2); mode = 1;
    } else {
        const float r = rng.nextf();
        if (r < P_ROT)                 { nrot = 1 - nrot; mode = 2; }
        else if (r < P_ROT + P_JUMP)   { mode = 1; }
        else if (rng.nextf() < P_SNAP) { mode = 3; }
        else                           { mode = 2; }
    }
    const int w = wof(i, nrot), h = hof(i, nrot);
    const int mx = BW - w, my = BH - h;
    if (mx < 0 || my < 0) { g_mv.non = 0; g_mv.nx = 0; g_mv.ny = 0; g_mv.nrot = nrot; return; }
    if (mode == 1) {
        nx = (int)rng.next((uint32_t)(mx + 1)); ny = (int)rng.next((uint32_t)(my + 1));
    } else if (mode == 2) {                       // 近くへ微移動
        nx += (int)rng.next((uint32_t)(2 * SHIFT_W + 1)) - SHIFT_W;
        ny += (int)rng.next((uint32_t)(2 * SHIFT_W + 1)) - SHIFT_W;
    } else if (mode == 3) {                       // 他の長方形の辺 (または盤の端) にぴったり寄せる
        const int j = (int)rng.next((uint32_t)N);
        const int k = (int)rng.next(4);
        if (k == 0)      nx = BX2[j];
        else if (k == 1) nx = BX[j] - w;
        else if (k == 2) ny = BY2[j];
        else             ny = BY[j] - h;
    }
    if (nx < 0) nx = 0; else if (nx > mx) nx = mx;
    if (ny < 0) ny = 0; else if (ny > my) ny = my;
    g_mv.nx = nx; g_mv.ny = ny; g_mv.non = non; g_mv.nrot = nrot;
}

inline float calc_score(State &s) {
    const int i = g_mv.i;
    const int sx = BX[i], sy = BY[i], sx2 = BX2[i], sy2 = BY2[i];
    BX[i] = BY[i] = BX2[i] = BY2[i] = 0;                    // 自分自身を数えないよう一時的に消す
    float d = 0.0f;
    if (g_mv.non)
        d += (float)RV[i] - OVER_PEN * (float)overlap_sum(g_mv.nx, g_mv.ny, wof(i, g_mv.nrot), hof(i, g_mv.nrot));
    if (s.on[i])
        d -= (float)RV[i] - OVER_PEN * (float)overlap_sum(sx, sy, sx2 - sx, sy2 - sy);
    BX[i] = sx; BY[i] = sy; BX2[i] = sx2; BY2[i] = sy2;
    return d;
}

inline void apply_move(State &s) {
    const int i = g_mv.i;
    s.x[i] = (int16_t)g_mv.nx; s.y[i] = (int16_t)g_mv.ny;
    s.rot[i] = (uint8_t)g_mv.nrot; s.on[i] = (uint8_t)g_mv.non;
    soa_set(s, i);
}

void read_input() {
    if (scanf("%d %d %d", &BW, &BH, &N) == 3 && N >= 1) {
        N = min(N, MAXN);
        for (int i = 0; i < N; i++) scanf("%d %d %d", &RW[i], &RH[i], &RV[i]);
    } else {
        uint32_t sd = 0; pick_env("SEED", sd);
        Xor128 g; g.seed(sd * 1000003u + 208);
        BW = 100; BH = 100; N = 60;
        for (int i = 0; i < N; i++) {
            RW[i] = 5 + (int)g.next(21); RH[i] = 5 + (int)g.next(21);
            RV[i] = (int)lroundf((float)(RW[i] * RH[i]) * (0.8f + 0.4f * g.nextf()));
        }
    }
}

void output(const State &s) {
    string r; r.reserve((size_t)N * 12);
    for (int i = 0; i < N; i++) {
        if (!s.on[i]) r += "-1 -1 -1\n";
        else { r += to_string((int)s.x[i]); r += ' '; r += to_string((int)s.y[i]); r += ' '; r += to_string((int)s.rot[i]); r += '\n'; }
    }
    fputs(r.c_str(), stdout);
}
// # ■ 問題ごとに書き換える部分 ここまで ■
// #############################################################################

// =============================================================================
//  採用判定用: log(一様乱数) のテーブル
// =============================================================================
//  採用条件  exp(gain / T) > u   (u は [0,1) の一様乱数)
//         ⇔ gain > T * log(u)   (log(u) <= 0)
//  この形にすると exp も log も除算も要らず「1 回の乗算と比較」だけで判定できる。
//  さらに gain >= 0 (改善) のときは右辺が負なので必ず真になり、分岐も 1 つで済む。
constexpr int      LOG_TBL_BITS = 12;
constexpr int      LOG_TBL_SIZE = 1 << LOG_TBL_BITS;      // 4096 個 = 16KB (L1 に載る)
constexpr uint32_t LOG_TBL_MASK = LOG_TBL_SIZE - 1;
static float g_log_tbl[LOG_TBL_SIZE];
void init_log_table() {
    Xor128 g; g.seed(1234567);
    for (int i = 0; i < LOG_TBL_SIZE; i++)
        g_log_tbl[i] = logf(((float)g.next() + 0.5f) * (1.0f / 4294967296.0f));
}

// =============================================================================
// 7. 焼きなまし法 本体
// =============================================================================
//  s を初期解として受け取り、deadline_ms まで焼きなまし、見つけた最良解を s に残す。
//  t0 : 開始温度 (これくらいの悪化なら受け入れる、という値。差分の最大値くらい)
//  t1 : 終了温度 (これ以上の悪化は受け入れない、という値。差分の最小値くらい)
void anneal(State &s, float deadline_ms, float t0, float t1) {
    static State best;                       // State が大きくても良いように static 領域へ
    if constexpr (KEEP_BEST) best = s;

    const float begin = timer.ms();
    const float span  = deadline_ms - begin;
    if (span <= 0.0f) return;
    if (t0 < 1e-9f) t0 = 1e-9f;              // 0 除算 / NaN 対策
    if (t1 < 1e-9f) t1 = 1e-9f;
    if (t1 > t0)    t1 = t0;                 // 念のため t0 >= t1 にしておく
    const float lr = logf(t1 / t0);          // 指数スケジュール用

    float temp    = t0;
    bool  keeping = (KEEP_BEST_FROM <= 0.0f);
    int   block   = ITER_PER_CHECK;          // ★ この回数に 1 回だけ時間計測する
    float prev    = begin;

    while (true) {
        // ---- 内側ループ: 時間計測も温度更新も一切しない (ここが最速であるべき) ----
        for (int it = 0; it < block; it++) {
            modify(s);                                  // 遷移を 1 つ選ぶ
            const float diff = calc_score(s);           // スコア差分
            const float gain = GAIN_SIGN * diff;        // 改善量 (正なら改善)
            // gain > T*log(u) : 改善なら必ず採用、悪化なら exp(gain/T) の確率で採用
            if (gain > temp * g_log_tbl[rng.next() & LOG_TBL_MASK]) {
                apply_move(s);                          // 採用時だけ実際に反映
                s.score += diff;
                g_accept++;
                if constexpr (KEEP_BEST) {
                    if (keeping && is_better(s.score, best.score)) best = s;
                }
            }
            // else rollback(s);   // ←「先に適用して却下時に戻す」方式にする場合はここ
        }
        g_iter += block;

        // ---- ここからブロックごとの処理 (時間計測・温度更新) ----
        const float now = timer.ms();
        if (now >= deadline_ms) break;
        const float r = (now - begin) / span;           // 進捗 [0,1)
        temp = t0 * expf(lr * r);                       // 指数スケジュール (既定)
        // temp = t0 + (t1 - t0) * r;                   // 線形スケジュール (こちらが良い問題もある)

        if constexpr (KEEP_BEST) {
            if (!keeping && r >= KEEP_BEST_FROM) {      // 途中から最良解の記録を始める (コピー削減)
                keeping = true;
                if (is_better(s.score, best.score)) best = s;
            }
        }
        tune_block(block, now - prev); prev = now;      // 1 ブロックの実行時間を一定に保つ
    }
    if constexpr (KEEP_BEST) { if (is_better(best.score, s.score)) s = best; }
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
    init_log_table();   // 採用判定用テーブル

    static State state; // State が大きくても良いように static 領域へ
    init_state(state);  // ★ 初期解構築
    anneal(state, TIME_LIMIT_MS, TEMP_START, TEMP_END);

    output(state);

    // ---- デバッグ出力 (stderr。不要なら消して良い) ----
    fprintf(stderr, "Score = %.0f\n", (double)state.score);
    fprintf(stderr, "iter = %lld, accept = %lld (%.2f%%), time = %.1f ms\n",
            g_iter, g_accept, 100.0 * (double)g_accept / (double)max(1LL, g_iter), (double)timer.ms());
    // ★差分計算のバグ検出: 下の 2 つがずれていたら calc_score / apply_move が間違っている
    fprintf(stderr, "[check] diff-sum = %.3f, full = %.3f\n",
            (double)state.score, (double)full_score(state));
    return 0;
}
