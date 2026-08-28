// =====================================================================================
//  RCO presents 日本橋ハーフマラソン 予選 A - Multiple Pieces
//  https://atcoder.jp/contests/rco-contest-2017-qual/tasks/rco_contest_2017_qual_a
//
//  H=50, W=50 のグリッドの各マスに 0..9 の数字。ちょうど K=8 マスの連結領域(ピース)を
//  マスを共有しないように好きなだけ作る。ピースのスコア = 含む K 個の数字の積。
//  合計の最大化 (提出得点は 合計 / 10000 の切り上げ)。
//
//  【解法】typical/polyomino のライブラリを使った 3 ステップ
//    1. K マスのポリオミノを全生成する (両面 369 種 -> 盤面に置ける向きに展開して 2725 通り)
//    2. 盤面へ置く方法を全列挙する。はみ出さず、0 を含まないものだけ残す
//       (0 を含むと積が 0 になり、そのピースは完全に無駄なので捨ててよい)
//    3. スコア(積)の大きい順にソートし、置けるなら置くを最後まで繰り返す
//
//  ビルド: g++-15 -std=c++20 -O2 -o a.out main.cpp
//  提出用 1 ファイル: python3 bundle.py   -> submit.cpp
// =====================================================================================
#include "../polyomino.hpp"

// ---------------------------------------------------------------- タイマー
struct Timer {
    chrono::steady_clock::time_point st = chrono::steady_clock::now();
    double ms() const { return chrono::duration<double, milli>(chrono::steady_clock::now() - st).count(); }
};
static Timer timer;

// ---------------------------------------------------------------- パラメータ
template <class T> inline void pick_env(const char *key, T &dst) {
    if (const char *s = getenv(key)) {
        char *e = nullptr;
        double v = strtod(s, &e);
        if (e != s) dst = (T)v;
    }
}

// p1: ポリオミノの数え方 0=有向(FIXED) / 1=片面(ONE_SIDED) / 2=両面(FREE)
//     ※ この問題ではピースを自由に回転・裏返しできるので、どれを選んでも
//        「盤面に置ける向き」に展開した結果は同じ(有向 2725 通り)になり、スコアは変わらない。
//        生成の単位が変わるだけ。裏返し禁止の問題では 1 を使う。
int SYM_MODE = 2;

void load_params() { pick_env("p1", SYM_MODE); }

// ---------------------------------------------------------------- 本体
int H, W, K;
vector<int> g;  // g[y*W+x] = 0..9

// 候補: score = 積, off = cells 配列中の先頭位置
struct Cand {
    uint32_t score;
    uint32_t off;
};

int main() {
    load_params();
    poly::Symmetry sym = (poly::Symmetry)max(0, min(2, SYM_MODE));

    // ---- 入力 (標準入力が無ければ環境変数 SEED から内部生成) ----
    if (!(cin >> H >> W >> K)) {
        H = 50; W = 50; K = 8;
        unsigned seed = 0;
        if (const char *s = getenv("SEED")) seed = (unsigned)atoi(s);
        mt19937 rng(seed);
        g.assign(H * W, 0);
        for (auto &v : g) v = (int)(rng() % 10);
    } else {
        g.assign(H * W, 0);
        for (int y = 0; y < H; y++) {
            string s;
            cin >> s;
            for (int x = 0; x < W; x++) g[y * W + x] = s[x] - '0';
        }
    }
    // 積を uint32 に収めるための前提 (9^9 = 387420489 < 2^32)
    if (K > 9) { fprintf(stderr, "K > 9 は uint32 の積に収まりません\n"); return 1; }

    // ================= 1. ポリオミノを生成する =================
    vector<poly::Shape> shapes = poly::generate(K, sym);
    vector<poly::Shape> ors    = poly::expand_orientations(shapes, sym);
    double t_gen = timer.ms();

    // ================= 2. 置き方を全列挙する (はみ出さない & 0 を含まない) =================
    // 置ける位置の総数を先に数えて確保しておく
    long long total_pos = 0;
    for (auto &s : ors) {
        int h = poly::height(s), w = poly::width(s);
        if (h <= H && w <= W) total_pos += (long long)(H - h + 1) * (W - w + 1);
    }
    vector<uint16_t> cells;
    vector<Cand> cand;
    cells.reserve((size_t)(total_pos * 0.5) * K);
    cand.reserve((size_t)(total_pos * 0.5));

    vector<int> off(K);
    for (auto &s : ors) {
        int h = poly::height(s), w = poly::width(s);
        if (h > H || w > W) continue;
        for (int i = 0; i < K; i++) off[i] = s[i].first * W + s[i].second;
        for (int oy = 0; oy + h <= H; oy++) {
            int row = oy * W;
            for (int ox = 0; ox + w <= W; ox++) {
                int base = row + ox;
                uint32_t prod = 1;
                int i = 0;
                for (; i < K; i++) {
                    int v = g[base + off[i]];
                    if (v == 0) break;  // 0 を含む配置は捨てる
                    prod *= (uint32_t)v;
                }
                if (i < K) continue;
                cand.push_back({prod, (uint32_t)cells.size()});
                for (int j = 0; j < K; j++) cells.push_back((uint16_t)(base + off[j]));
            }
        }
    }
    double t_enum = timer.ms();

    // ================= 3. スコアの大きい順に、置けるなら置く =================
    sort(cand.begin(), cand.end(), [](const Cand &a, const Cand &b) { return a.score > b.score; });
    double t_sort = timer.ms();

    vector<char> used(H * W, 0);
    vector<uint32_t> chosen;
    long long total = 0;
    for (const Cand &c : cand) {
        const uint16_t *p = &cells[c.off];
        int i = 0;
        for (; i < K; i++) if (used[p[i]]) break;
        if (i < K) continue;
        for (int j = 0; j < K; j++) used[p[j]] = 1;
        chosen.push_back(c.off);
        total += c.score;
    }

    // ---- 出力 (1-indexed) ----
    {
        string out;
        out.reserve(chosen.size() * K * 8 + 16);
        out += to_string(chosen.size());
        out += '\n';
        for (uint32_t o : chosen)
            for (int j = 0; j < K; j++) {
                int p = cells[o + j];
                out += to_string(p / W + 1);
                out += ' ';
                out += to_string(p % W + 1);
                out += '\n';
            }
        fwrite(out.data(), 1, out.size(), stdout);
    }

    // ---- 検証 (バグ検出用) ----
    {
        vector<char> seen(H * W, 0);
        long long replay = 0;
        bool ok = true;
        for (uint32_t o : chosen) {
            poly::Shape sh;
            long long prod = 1;
            for (int j = 0; j < K; j++) {
                int p = cells[o + j];
                if (seen[p]) ok = false;  // マスの使い回し
                seen[p] = 1;
                prod *= g[p];
                sh.push_back({p / W, p % W});
            }
            if ((int)sh.size() != K || !poly::is_connected(poly::normalize(sh))) ok = false;
            replay += prod;
        }
        fprintf(stderr, "[check] pieces = %zu, running = %lld / replay = %lld, valid = %s\n",
                chosen.size(), total, replay, (ok && replay == total) ? "OK" : "*** NG ***");
    }
    fprintf(stderr, "sym = %s, shapes = %zu, orientations = %zu\n", poly::sym_name(sym), shapes.size(), ors.size());
    fprintf(stderr, "placements: %lld tried -> %zu kept (0 を含まないもの)\n", total_pos, cand.size());
    fprintf(stderr, "time: gen %.0f ms, enum %.0f ms, sort %.0f ms, total %.0f ms\n",
            t_gen, t_enum - t_gen, t_sort - t_enum, timer.ms());
    fprintf(stderr, "sum = %lld\n", total);
    fprintf(stderr, "Score = %lld\n", (total + 9999) / 10000);
    return 0;
}
