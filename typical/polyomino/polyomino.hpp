#pragma once
// =====================================================================================
//  ポリオミノ生成ライブラリ  (AHC Library / typical)
//  参考: https://ja.wikipedia.org/wiki/ポリオミノ
//
//  K 個の正方形からなるポリオミノを「有向 / 片面 / 両面」を切り替えて全列挙する。
//    poly::FIXED     有向 (fixed)     : 平行移動のみで同一視。回転・裏返しは別物
//    poly::ONE_SIDED 片面 (one-sided) : 平行移動 + 回転で同一視。裏返しは別物
//    poly::FREE      両面 (free)      : 平行移動 + 回転 + 裏返しで同一視
//
//    K      1  2  3   4   5   6    7    8     9     10
//    両面   1  1  2   5  12  35  108  369  1285   4655
//    片面   1  1  2   7  18  60  196  704  2500   9189
//    有向   1  2  6  19  63 216  760 2725  9910  36446
//
//  生成は Redelmeier のアルゴリズム(有向を重複なく直接列挙)＋正規形による重複除去。
//  実用域は K <= 12 程度 (有向で 505861 個)。
// =====================================================================================
#include <bits/stdc++.h>
using namespace std;

namespace poly {

enum Symmetry { FIXED = 0, ONE_SIDED = 1, FREE = 2 };

inline const char* sym_name(Symmetry s) {
    return s == FIXED ? "FIXED(有向)" : s == ONE_SIDED ? "ONE_SIDED(片面)" : "FREE(両面)";
}

using Cell  = pair<int, int>;  // (y, x)
// Shape は「正規化済み」を不変条件とする: 昇順ソート済み かつ min(y)=min(x)=0
using Shape = vector<Cell>;

// ---------------------------------------------------------------- 基本操作

inline Shape normalize(Shape s) {
    int my = INT_MAX, mx = INT_MAX;
    for (auto& c : s) { my = min(my, c.first); mx = min(mx, c.second); }
    for (auto& c : s) { c.first -= my; c.second -= mx; }
    sort(s.begin(), s.end());
    return s;
}

inline int height(const Shape& s) {
    int h = 0; for (auto& c : s) h = max(h, c.first);  return h + 1;
}
inline int width(const Shape& s) {
    int w = 0; for (auto& c : s) w = max(w, c.second); return w + 1;
}

// 二面体群 D4 の 8 要素。t&4 で転置、t&1 で y 反転、t&2 で x 反転。
// 回転のみ(行列式 +1)は t = 0, 3, 5, 6 の 4 つ。
inline const array<int, 4>& rot_ids() { static const array<int, 4> v{0, 3, 5, 6}; return v; }
inline const array<int, 8>& all_ids() { static const array<int, 8> v{0, 1, 2, 3, 4, 5, 6, 7}; return v; }

inline Shape transform(const Shape& s, int t) {
    Shape r(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        int y = s[i].first, x = s[i].second;
        if (t & 4) swap(y, x);
        if (t & 1) y = -y;
        if (t & 2) x = -x;
        r[i] = {y, x};
    }
    return normalize(move(r));
}

// 対称性 sym のもとで shape が取りうる「盤面に置ける向き」を重複なく返す。
//   FIXED     -> 1 通り (そのまま)
//   ONE_SIDED -> 回転 4 通りのうち相異なるもの
//   FREE      -> 回転 + 裏返し 8 通りのうち相異なるもの
inline vector<Shape> orientations(const Shape& s, Symmetry sym) {
    vector<Shape> res;
    auto add = [&](int t) {
        Shape c = transform(s, t);
        for (auto& r : res) if (r == c) return;
        res.push_back(move(c));
    };
    if (sym == FIXED)          add(0);
    else if (sym == ONE_SIDED) for (int t : rot_ids()) add(t);
    else                       for (int t : all_ids()) add(t);
    return res;
}

// 同値類の代表元 (取りうる向きのうち辞書順最小)
inline Shape canonical(const Shape& s, Symmetry sym) {
    if (sym == FIXED) return normalize(s);
    Shape best;
    auto upd = [&](int t) { Shape c = transform(s, t); if (best.empty() || c < best) best = move(c); };
    if (sym == ONE_SIDED) for (int t : rot_ids()) upd(t);
    else                  for (int t : all_ids()) upd(t);
    return best;
}

inline bool is_connected(const Shape& s) {
    if (s.empty()) return false;
    set<Cell> st(s.begin(), s.end());
    vector<Cell> stk{s[0]};
    set<Cell> vis{s[0]};
    const int dy[4] = {-1, 1, 0, 0}, dx[4] = {0, 0, -1, 1};
    while (!stk.empty()) {
        auto [y, x] = stk.back(); stk.pop_back();
        for (int d = 0; d < 4; d++) {
            Cell n{y + dy[d], x + dx[d]};
            if (st.count(n) && !vis.count(n)) { vis.insert(n); stk.push_back(n); }
        }
    }
    return vis.size() == s.size();
}

// ---------------------------------------------------------------- 生成 (Redelmeier)

namespace detail {

// 有向ポリオミノを重複なく 1 個ずつ列挙する。
// 「行優先で最小のマスを原点に置く」正規位置に限定することで重複を防ぐ。
struct FixedGen {
    int K = 0, W2 = 0, NC = 0, id0 = 0;
    vector<char> mark;   // そのマスが poly か untried に入ったことがあるか
    vector<int>  cur;
    function<void(const Shape&)> emit;

    void run(int k, const function<void(const Shape&)>& f) {
        if (k <= 0) return;
        K = k; W2 = 2 * K + 1; NC = (K + 1) * W2; id0 = K;  // 原点 = (y=0, x=0)
        mark.assign(NC, 0);
        cur.clear(); cur.reserve(K);
        emit = f;
        mark[id0] = 1;
        extend(vector<int>{id0});
    }

    void extend(vector<int> untried) {
        while (!untried.empty()) {
            int c = untried.back(); untried.pop_back();
            cur.push_back(c);
            if ((int)cur.size() == K) {
                Shape s(K);
                for (int i = 0; i < K; i++) s[i] = {cur[i] / W2, cur[i] % W2 - K};
                emit(normalize(move(s)));
            } else {
                vector<int> child = untried, newly;
                int y = c / W2, xi = c % W2;
                int cand[4] = {y > 0      ? c - W2 : -1,
                               y < K      ? c + W2 : -1,
                               xi > 0     ? c - 1  : -1,
                               xi + 1 < W2 ? c + 1  : -1};
                for (int d : cand)
                    if (d >= id0 && !mark[d]) { mark[d] = 1; child.push_back(d); newly.push_back(d); }
                extend(move(child));
                for (int d : newly) mark[d] = 0;
            }
            cur.pop_back();
            // c は mark されたまま残す = この階層で二度と使わない (重複防止)
        }
    }
};

inline string encode(const Shape& s) {
    string k(s.size() * 2, '\0');
    for (size_t i = 0; i < s.size(); i++) {
        k[2 * i]     = (char)(s[i].first  + 1);
        k[2 * i + 1] = (char)(s[i].second + 1);
    }
    return k;
}

}  // namespace detail

// K マスのポリオミノを sym の意味で全列挙する (辞書順ソート済み)。
inline vector<Shape> generate(int K, Symmetry sym) {
    vector<Shape> res;
    if (K <= 0) return res;
    detail::FixedGen g;
    if (sym == FIXED) {
        g.run(K, [&](const Shape& s) { res.push_back(s); });
    } else {
        unordered_set<string> seen;
        g.run(K, [&](const Shape& s) {
            Shape c = canonical(s, sym);
            if (seen.insert(detail::encode(c)).second) res.push_back(move(c));
        });
    }
    sort(res.begin(), res.end());
    return res;
}

// 1..K マスをまとめて生成 (res[n] が n マスの一覧、res[0] は空)
inline vector<vector<Shape>> generate_upto(int K, Symmetry sym) {
    vector<vector<Shape>> res(K + 1);
    for (int n = 1; n <= K; n++) res[n] = generate(n, sym);
    return res;
}

// ---------------------------------------------------------------- 盤面への配置

// H x W の盤面に shape を平行移動して置ける全パターンを列挙し、
// 平行移動後のマス列を cb(const Shape&) に渡す。回転・裏返しは orientations() で先に展開すること。
template <class F>
inline void for_each_placement(int H, int W, const Shape& s, F cb) {
    int h = height(s), w = width(s);
    if (h > H || w > W) return;
    Shape t(s.size());
    for (int oy = 0; oy + h <= H; oy++)
        for (int ox = 0; ox + w <= W; ox++) {
            for (size_t i = 0; i < s.size(); i++) t[i] = {s[i].first + oy, s[i].second + ox};
            cb(t);
        }
}

// sym で列挙した shapes を「盤面に置ける向き」に展開して重複を除いた一覧を返す。
// (数学的には sym によらず有向ポリオミノ全体と一致する。裏返し禁止の問題で
//  「1 種類につき何通りの向きがあるか」を数えたいときに orientations() を使う)
inline vector<Shape> expand_orientations(const vector<Shape>& shapes, Symmetry sym) {
    vector<Shape> res;
    unordered_set<string> seen;
    for (auto& s : shapes)
        for (auto& o : orientations(s, sym))
            if (seen.insert(detail::encode(o)).second) res.push_back(o);
    sort(res.begin(), res.end());
    return res;
}

// ---------------------------------------------------------------- 入出力補助

inline vector<string> to_ascii(const Shape& s, char on = '#', char off = '.') {
    vector<string> g(height(s), string(width(s), off));
    for (auto& c : s) g[c.first][c.second] = on;
    return g;
}

inline Shape from_ascii(const vector<string>& g, char off = '.') {
    Shape s;
    for (int y = 0; y < (int)g.size(); y++)
        for (int x = 0; x < (int)g[y].size(); x++)
            if (g[y][x] != off && !isspace((unsigned char)g[y][x])) s.push_back({y, x});
    return normalize(move(s));
}

// 複数の形を横に並べて stderr 等に出す
inline string tile_ascii(const vector<Shape>& v, int cols = 10, char on = '#', char off = '.') {
    string out;
    for (size_t i = 0; i < v.size(); i += cols) {
        size_t j = min(v.size(), i + (size_t)cols);
        int h = 0;
        for (size_t k = i; k < j; k++) h = max(h, height(v[k]));
        vector<vector<string>> a;
        for (size_t k = i; k < j; k++) a.push_back(to_ascii(v[k], on, off));
        for (int r = 0; r < h; r++) {
            for (size_t k = 0; k < a.size(); k++) {
                int w = (int)a[k][0].size();
                out += (r < (int)a[k].size()) ? a[k][r] : string(w, ' ');
                out += "  ";
            }
            out += '\n';
        }
        out += '\n';
    }
    return out;
}

}  // namespace poly
