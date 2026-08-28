// ==========================================================
// セグメントツリー (一点更新・区間クエリ) 全パターン チートシート
// ==========================================================
// 汎用テンプレート (segment_tree.cpp) に渡す op, e の定義一覧
//
// テンプレート: SegTree<T, op, e>
//   set(p, x):     a[p] = x
//   update(p, x):  a[p] = op(a[p], x)
//   query(l, r):   op(a[l], a[l+1], ..., a[r-1])
// ==========================================================
#include <bits/stdc++.h>
using namespace std;

static constexpr long long INF = 1e18;
static constexpr long long MOD = 998244353;

// ==========================================================
// 1. 区間和 (Range Sum)
//    set + query で一点代入 + 区間和
//    update + query で一点加算 + 区間和
// ==========================================================
namespace RangeSum {
    using T = long long;
    T op(T a, T b){ return a + b; }
    T e(){ return 0; }
}

// ==========================================================
// 2. 区間最小値 (Range Min / RMQ)
//    set + query で一点代入 + 区間min
// ==========================================================
namespace RangeMin {
    using T = long long;
    T op(T a, T b){ return min(a, b); }
    T e(){ return INF; }
}

// ==========================================================
// 3. 区間最大値 (Range Max)
// ==========================================================
namespace RangeMax {
    using T = long long;
    T op(T a, T b){ return max(a, b); }
    T e(){ return -INF; }
}

// ==========================================================
// 4. 区間XOR
//    update(p, x) で一点XOR更新, query(l, r) で区間XOR
// ==========================================================
namespace RangeXor {
    using T = long long;
    T op(T a, T b){ return a ^ b; }
    T e(){ return 0; }
}

// ==========================================================
// 5. 区間GCD
//    gcd(0, x) = x なので単位元は 0
// ==========================================================
namespace RangeGcd {
    using T = long long;
    T op(T a, T b){ return __gcd(a, b); }
    T e(){ return 0; }
}

// ==========================================================
// 6. 区間積 (mod)
// ==========================================================
namespace RangeProduct {
    using T = long long;
    T op(T a, T b){ return a % MOD * (b % MOD) % MOD; }
    T e(){ return 1; }
}

// ==========================================================
// 7. 区間 Bitwise OR
// ==========================================================
namespace RangeOr {
    using T = long long;
    T op(T a, T b){ return a | b; }
    T e(){ return 0; }
}

// ==========================================================
// 8. 区間 Bitwise AND
// ==========================================================
namespace RangeAnd {
    using T = long long;
    T op(T a, T b){ return a & b; }
    T e(){ return ~0LL; } // 全ビット1 (ANDの単位元)
}

// ==========================================================
// 9. 区間 (最大値, 個数)  [複合型]
//    最大値とその出現回数を同時に管理
// ==========================================================
namespace RangeMaxCount {
    struct T { long long mx; int cnt; };
    T op(T a, T b){
        if(a.mx > b.mx) return a;
        if(a.mx < b.mx) return b;
        return {a.mx, a.cnt + b.cnt};
    }
    T e(){ return {-INF, 0}; }
    // 初期化: {a[i], 1}
}

// ==========================================================
// 10. 区間 (最小値, インデックス)  [複合型]
//     最小値とそのインデックスを同時に管理
// ==========================================================
namespace RangeMinIndex {
    struct T { long long val; int idx; };
    T op(T a, T b){ return a.val <= b.val ? a : b; }
    T e(){ return {INF, -1}; }
    // 初期化: {a[i], i}
}

// ==========================================================
// 11. 区間 (最小値, 個数)  [複合型]
// ==========================================================
namespace RangeMinCount {
    struct T { long long mn; int cnt; };
    T op(T a, T b){
        if(a.mn < b.mn) return a;
        if(a.mn > b.mn) return b;
        return {a.mn, a.cnt + b.cnt};
    }
    T e(){ return {INF, 0}; }
}

// ==========================================================
// 12. 区間 (和, 長さ)
//     遅延セグ木の区間加算+区間和で使うS型と同じ
//     通常セグ木でも使うことがある
// ==========================================================
namespace RangeSumSize {
    struct T { long long sum; int size; };
    T op(T a, T b){ return {a.sum + b.sum, a.size + b.size}; }
    T e(){ return {0, 0}; }
    // 初期化: {a[i], 1}
}

// ==========================================================
// まとめ表
// ==========================================================
//
//  #  | 型         | op                | e()        | 用途
// ----+------------+-------------------+------------+------------------
//  1  | ll         | a + b             | 0          | 区間和
//  2  | ll         | min(a, b)         | INF        | 区間min (RMQ)
//  3  | ll         | max(a, b)         | -INF       | 区間max
//  4  | ll         | a ^ b             | 0          | 区間XOR
//  5  | ll         | gcd(a, b)         | 0          | 区間GCD
//  6  | ll         | a * b % MOD       | 1          | 区間積 (mod)
//  7  | ll         | a | b             | 0          | 区間OR
//  8  | ll         | a & b             | ~0         | 区間AND
//  9  | {mx, cnt}  | max合算            | {-INF, 0}  | 最大値+個数
// 10  | {val, idx} | min選択            | {INF, -1}  | 最小値+位置
// 11  | {mn, cnt}  | min合算            | {INF, 0}   | 最小値+個数
// 12  | {sum, sz}  | 和合算             | {0, 0}     | 和+サイズ

int main(){
    // 使用例: SegTree<RangeSum::T, RangeSum::op, RangeSum::e> seg(N);
    return 0;
}
