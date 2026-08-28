// ==========================================================
// 遅延セグメントツリー 全パターン チートシート
// ==========================================================
// 汎用テンプレート (lazy_segment_tree.cpp) に渡す
// op, e, mapping, composition, id の定義一覧
//
// テンプレート:
//   LazySegTree<S, op, e, F, mapping, composition, id>
//
// S: ノードが持つ値の型 (モノイド)
// F: 遅延作用の型
// op(S,S)->S:           2つの子をマージ
// e()->S:               モノイドの単位元
// mapping(F,S)->S:      遅延作用をノードに適用
// composition(F,F)->F:  遅延作用の合成 (新しい作用, 古い作用)
// id()->F:              作用の単位元 (何もしない)
// ==========================================================
#include <bits/stdc++.h>
using namespace std;

static constexpr long long INF = 1e18;
static constexpr long long MOD = 998244353;

// ==========================================================
// 1. 区間加算 + 区間和
//    apply(l,r,v): a[i] += v
//    query(l,r):   Σa[i]
// ==========================================================
namespace RangeAddRangeSum {
    struct S { long long sum; int size; };
    using F = long long;
    S op(S a, S b){ return {a.sum + b.sum, a.size + b.size}; }
    S e(){ return {0, 0}; }
    S mapping(F f, S s){ return {s.sum + f * s.size, s.size}; }
    F composition(F f, F g){ return f + g; }
    F id(){ return 0; }
    // 初期化: init[i] = {a[i], 1}
}

// ==========================================================
// 2. 区間加算 + 区間最小値
//    apply(l,r,v): a[i] += v
//    query(l,r):   min(a[i])
// ==========================================================
namespace RangeAddRangeMin {
    using S = long long;
    using F = long long;
    S op(S a, S b){ return min(a, b); }
    S e(){ return INF; }
    S mapping(F f, S s){ return s + f; }
    F composition(F f, F g){ return f + g; }
    F id(){ return 0; }
    // 初期化: init[i] = a[i]
}

// ==========================================================
// 3. 区間加算 + 区間最大値
//    apply(l,r,v): a[i] += v
//    query(l,r):   max(a[i])
// ==========================================================
namespace RangeAddRangeMax {
    using S = long long;
    using F = long long;
    S op(S a, S b){ return max(a, b); }
    S e(){ return -INF; }
    S mapping(F f, S s){ return s + f; }
    F composition(F f, F g){ return f + g; }
    F id(){ return 0; }
}

// ==========================================================
// 4. 区間代入 + 区間和
//    apply(l,r,v): a[i] = v
//    query(l,r):   Σa[i]
// ==========================================================
namespace RangeUpdateRangeSum {
    struct S { long long sum; int size; };
    static constexpr long long NONE = -1e18;
    using F = long long;
    S op(S a, S b){ return {a.sum + b.sum, a.size + b.size}; }
    S e(){ return {0, 0}; }
    S mapping(F f, S s){ return f == NONE ? s : S{f * s.size, s.size}; }
    F composition(F f, F g){ return f == NONE ? g : f; }
    F id(){ return NONE; }
    // 初期化: init[i] = {a[i], 1}
}

// ==========================================================
// 5. 区間代入 + 区間最小値
//    apply(l,r,v): a[i] = v
//    query(l,r):   min(a[i])
// ==========================================================
namespace RangeUpdateRangeMin {
    using S = long long;
    static constexpr long long NONE = -1e18;
    using F = long long;
    S op(S a, S b){ return min(a, b); }
    S e(){ return INF; }
    S mapping(F f, S s){ return f == NONE ? s : f; }
    F composition(F f, F g){ return f == NONE ? g : f; }
    F id(){ return NONE; }
}

// ==========================================================
// 6. 区間代入 + 区間最大値
//    apply(l,r,v): a[i] = v
//    query(l,r):   max(a[i])
// ==========================================================
namespace RangeUpdateRangeMax {
    using S = long long;
    static constexpr long long NONE = -1e18;
    using F = long long;
    S op(S a, S b){ return max(a, b); }
    S e(){ return -INF; }
    S mapping(F f, S s){ return f == NONE ? s : f; }
    F composition(F f, F g){ return f == NONE ? g : f; }
    F id(){ return NONE; }
}

// ==========================================================
// 7. 区間アフィン変換 + 区間和  (最強パターン)
//    apply(l,r,{a,b}): a[i] = a * a[i] + b
//    query(l,r):        Σa[i]
//    ※ 区間加算 = apply(l,r,{1,v})
//    ※ 区間代入 = apply(l,r,{0,v})
//    ※ 区間乗算 = apply(l,r,{v,0})
// ==========================================================
namespace RangeAffineRangeSum {
    struct S { long long sum; int size; };
    struct F {
        long long a, b;
        bool operator==(const F& o) const { return a == o.a && b == o.b; }
    };
    S op(S x, S y){ return {(x.sum + y.sum) % MOD, x.size + y.size}; }
    S e(){ return {0, 0}; }
    S mapping(F f, S s){
        return {(f.a % MOD * s.sum + f.b % MOD * s.size) % MOD, s.size};
    }
    F composition(F f, F g){
        return {f.a * g.a % MOD, (f.a * g.b + f.b) % MOD};
    }
    F id(){ return {1, 0}; }
    // 初期化: init[i] = {a[i] % MOD, 1}
}

// ==========================================================
// 8. 区間加算 + 区間GCD
//    apply(l,r,v): a[i] += v
//    query(l,r):   gcd(a[i])
//    ※ 差分列d[i]=a[i+1]-a[i]のGCD + a[l] で管理するテクニック
//       gcd(a[l..r)) = gcd(a[l], gcd(d[l..r-1)))
//       d列は一点更新なので通常のSegTreeでOK (遅延不要)
// ==========================================================
// → これは遅延セグ木ではなく、差分セグ木+BIT の組み合わせで解くのが定石
//    参考: https://codeforces.com/blog/entry/18051

// ==========================================================
// まとめ表
// ==========================================================
//
// 作用 \ クエリ  | Sum (和)              | Min (最小)           | Max (最大)
// ---------------+-----------------------+----------------------+---------------------
// 区間加算       | 1. RangeAddRangeSum   | 2. RangeAddRangeMin  | 3. RangeAddRangeMax
// 区間代入       | 4. RangeUpdateRangeSum| 5. RangeUpdateRangeMin| 6. RangeUpdateRangeMax
// 区間アフィン   | 7. RangeAffineRangeSum| (不要: 加算or代入で十分) | (同左)
//
// 最も汎用的なのは 7 (アフィン+和)。加算・代入・乗算を全て包含する。
// アフィンが不要なら、1~6 の特殊版を使うとコードが簡潔。

int main(){
    // 使用例: RangeAddRangeSum
    using namespace RangeAddRangeSum;
    // LazySegTree<S, op, e, F, mapping, composition, id> seg(init);
    return 0;
}
