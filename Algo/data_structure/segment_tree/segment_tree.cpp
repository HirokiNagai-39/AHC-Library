// セグメントツリー (一点更新・区間クエリ) 汎用テンプレート
// 計算量: 構築 O(N), 更新 O(log N), クエリ O(log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Segment Tree ===
// T:     データ型 (モノイドの要素)
// op:    T × T → T  (結合的な二項演算. 例: min, max, +, ^, gcd)
// e:     () → T     (opの単位元. 例: INF, -INF, 0, 0, 0)
template<typename T, T (*op)(T, T), T (*e)()>
struct SegTree {
    int n;
    vector<T> dat;

    // --- コンストラクタ ---
    // (1) サイズ指定: 全要素を e() で初期化
    SegTree(int n_) : n(1) {
        while(n < n_) n <<= 1;
        dat.assign(2 * n, e());
    }
    // (2) 配列から構築: O(N)
    SegTree(const vector<T>& v) : SegTree((int)v.size()) {
        for(int i = 0; i < (int)v.size(); i++) dat[n + i] = v[i];
        for(int i = n - 1; i >= 1; i--) dat[i] = op(dat[2*i], dat[2*i+1]);
    }

    // --- set(p, x) ---
    // a[p] を x に置き換える (代入)
    // 用途: 値の完全な上書き
    void set(int p, T x){
        p += n;
        dat[p] = x;
        while(p >>= 1) dat[p] = op(dat[2*p], dat[2*p+1]);
    }

    // --- update(p, x) ---
    // a[p] = op(a[p], x)  (既存値と x を演算で結合)
    // 用途: 加算(sum), XOR更新(xor), etc.
    void update(int p, T x){
        p += n;
        dat[p] = op(dat[p], x);
        while(p >>= 1) dat[p] = op(dat[2*p], dat[2*p+1]);
    }

    // --- query(l, r) ---
    // op(a[l], a[l+1], ..., a[r-1]) を返す (半開区間 [l, r))
    // 用途: 区間和, 区間min, 区間max, 区間XOR, 区間GCD, etc.
    T query(int l, int r){
        T vl = e(), vr = e();
        for(l += n, r += n; l < r; l >>= 1, r >>= 1){
            if(l & 1) vl = op(vl, dat[l++]);
            if(r & 1) vr = op(dat[--r], vr);
        }
        return op(vl, vr);
    }

    // --- operator[](p) ---
    // a[p] の現在値を取得
    T operator[](int p){ return dat[p + n]; }

    // --- max_right(l, f) ---
    // f(op(a[l], ..., a[r-1])) = true となる最大の r を返す
    // f(e()) = true が必要. f は単調 (true...true, false...false)
    // 用途: 累積和が閾値を超える位置, etc.
    template<typename F>
    int max_right(int l, F f){
        if(l == n) return n; // 実際の要素数でなくnを使う点に注意
        l += n;
        T sm = e();
        do {
            while(l % 2 == 0) l >>= 1;
            if(!f(op(sm, dat[l]))){
                while(l < n){
                    l = 2 * l;
                    if(f(op(sm, dat[l]))){
                        sm = op(sm, dat[l]);
                        l++;
                    }
                }
                return l - n;
            }
            sm = op(sm, dat[l]);
            l++;
        } while((l & -l) != l);
        return n; // 全区間でtrue
    }

    // --- min_left(r, f) ---
    // f(op(a[l], ..., a[r-1])) = true となる最小の l を返す
    // f(e()) = true が必要. f は単調
    template<typename F>
    int min_left(int r, F f){
        if(r == 0) return 0;
        r += n;
        T sm = e();
        do {
            r--;
            while(r > 1 && (r % 2)) r >>= 1;
            if(!f(op(dat[r], sm))){
                while(r < n){
                    r = 2 * r + 1;
                    if(f(op(dat[r], sm))){
                        sm = op(dat[r], sm);
                        r--;
                    }
                }
                return r + 1 - n;
            }
            sm = op(dat[r], sm);
        } while((r & -r) != r);
        return 0;
    }
};

// ==========================================================
// 演算の定義例
// ==========================================================

// --- 区間和 ---
long long op_sum(long long a, long long b){ return a + b; }
long long e_sum(){ return 0; }

// --- 区間最小値 (RMQ) ---
long long op_min(long long a, long long b){ return min(a, b); }
long long e_min(){ return 1e18; }

// --- 区間最大値 ---
long long op_max(long long a, long long b){ return max(a, b); }
long long e_max(){ return -1e18; }

// --- 区間XOR ---
long long op_xor(long long a, long long b){ return a ^ b; }
long long e_xor(){ return 0; }

// --- 区間GCD ---
long long op_gcd(long long a, long long b){ return __gcd(a, b); }
long long e_gcd(){ return 0; }

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    int N = 5;
    vector<long long> a = {3, 1, 4, 1, 5};

    // ============ 区間和セグ木 ============
    {
        SegTree<long long, op_sum, e_sum> seg(a);  // 配列から構築

        // query(l, r): 半開区間 [l, r) の和
        cout << seg.query(0, 5) << "\n"; // 3+1+4+1+5 = 14
        cout << seg.query(1, 4) << "\n"; // 1+4+1 = 6

        // set(p, x): a[p] = x (代入)
        seg.set(2, 10);                  // a = {3, 1, 10, 1, 5}
        cout << seg.query(0, 5) << "\n"; // 3+1+10+1+5 = 20

        // update(p, x): a[p] += x (加算)
        seg.update(0, 7);                // a = {10, 1, 10, 1, 5}
        cout << seg.query(0, 5) << "\n"; // 10+1+10+1+5 = 27

        // operator[](p): a[p] の値を取得
        cout << seg[0] << "\n";          // 10
        cout << seg[2] << "\n";          // 10
    }

    // ============ 区間最小値セグ木 (RMQ) ============
    {
        SegTree<long long, op_min, e_min> seg(a);

        cout << seg.query(0, 5) << "\n"; // min(3,1,4,1,5) = 1
        cout << seg.query(2, 5) << "\n"; // min(4,1,5) = 1

        seg.set(3, 100);                 // a = {3, 1, 4, 100, 5}
        cout << seg.query(0, 5) << "\n"; // min = 1
        cout << seg.query(2, 5) << "\n"; // min(4,100,5) = 4
    }

    // ============ 区間最大値セグ木 ============
    {
        SegTree<long long, op_max, e_max> seg(a);

        cout << seg.query(0, 5) << "\n"; // max(3,1,4,1,5) = 5
        seg.set(1, 99);
        cout << seg.query(0, 3) << "\n"; // max(3,99,4) = 99
    }

    // ============ 区間XORセグ木 ============
    {
        SegTree<long long, op_xor, e_xor> seg(a);

        cout << seg.query(0, 5) << "\n"; // 3^1^4^1^5 = 2

        // update(p, x): a[p] ^= x (XOR更新)
        seg.update(0, 3);                // a[0] ^= 3 → 3^3 = 0
        cout << seg.query(0, 5) << "\n"; // 0^1^4^1^5 = 1
    }

    // ============ 区間GCDセグ木 ============
    {
        vector<long long> b = {12, 18, 24, 36};
        SegTree<long long, op_gcd, e_gcd> seg(b);

        cout << seg.query(0, 4) << "\n"; // gcd(12,18,24,36) = 6
        cout << seg.query(0, 2) << "\n"; // gcd(12,18) = 6
        cout << seg.query(1, 3) << "\n"; // gcd(18,24) = 6
    }

    // ============ サイズ指定コンストラクタ ============
    {
        SegTree<long long, op_sum, e_sum> seg(10); // 全要素0
        seg.set(3, 100);
        seg.set(7, 200);
        cout << seg.query(0, 10) << "\n"; // 300
    }

    // ============ max_right / min_left (セグ木上の二分探索) ============
    {
        // a = {3, 1, 4, 1, 5, 9, 2, 6}
        vector<long long> b = {3, 1, 4, 1, 5, 9, 2, 6};
        SegTree<long long, op_sum, e_sum> seg(b);

        // max_right(l, f): f(sum(a[l..r))) が true となる最大の r
        // 例: 位置0から累積和が10未満の最大区間
        int r = seg.max_right(0, [](long long x){ return x < 10; });
        cout << r << "\n"; // 4 (3+1+4+1=9 < 10, 3+1+4+1+5=14 >= 10)

        // min_left(r, f): f(sum(a[l..r))) が true となる最小の l
        // 例: 位置8(末尾)から左へ累積和が10未満の最小区間
        int l = seg.min_left(8, [](long long x){ return x < 10; });
        cout << l << "\n"; // 6 (2+6=8 < 10, 9+2+6=17 >= 10)
    }

    return 0;
}
