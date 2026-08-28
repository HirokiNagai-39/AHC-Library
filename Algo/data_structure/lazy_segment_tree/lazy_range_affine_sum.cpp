// 遅延セグメントツリー: 区間アフィン変換 + 区間和 (最強パターン)
// apply(l, r, {a, b}): a[i] = a * a[i] + b  for i in [l, r)
//   区間加算 = apply(l, r, {1, v})    (a[i] += v)
//   区間代入 = apply(l, r, {0, v})    (a[i] = v)
//   区間乗算 = apply(l, r, {v, 0})    (a[i] *= v)
// query(l, r): Σa[l..r) mod MOD
// 例題: Library Checker - Range Affine Range Sum
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 998244353;

struct S { long long sum; int size; };
struct F {
    long long a, b;
    bool operator==(const F& o) const { return a == o.a && b == o.b; }
};

S op(S x, S y){ return {(x.sum + y.sum) % MOD, x.size + y.size}; }
S e(){ return {0, 0}; }
S mapping(F f, S s){
    return {(f.a % MOD * s.sum % MOD + f.b % MOD * s.size % MOD) % MOD, s.size};
}
F composition(F f, F g){
    return {f.a * g.a % MOD, (f.a * g.b + f.b) % MOD};
}
F id(){ return {1, 0}; }

struct LazySegTree {
    int n, log_;
    vector<S> dat;
    vector<F> lz;

    LazySegTree(int n_) : n(1), log_(0) {
        while(n < n_){ n <<= 1; log_++; }
        dat.assign(2*n, e()); lz.assign(n, id());
    }
    LazySegTree(const vector<S>& v) : LazySegTree((int)v.size()) {
        for(int i = 0; i < (int)v.size(); i++) dat[n+i] = v[i];
        for(int i = n-1; i >= 1; i--) dat[i] = op(dat[2*i], dat[2*i+1]);
    }

    void push(int k){
        if(lz[k] == id()) return;
        dat[2*k] = mapping(lz[k], dat[2*k]);
        dat[2*k+1] = mapping(lz[k], dat[2*k+1]);
        if(2*k < n){ lz[2*k] = composition(lz[k], lz[2*k]); lz[2*k+1] = composition(lz[k], lz[2*k+1]); }
        lz[k] = id();
    }
    void push_down(int p){ for(int i = log_; i >= 1; i--) push(p >> i); }
    void push_up(int p){ while(p >>= 1) dat[p] = op(dat[2*p], dat[2*p+1]); }

    // --- apply(l, r, f) ---
    // a[i] = f.a * a[i] + f.b  for i in [l, r)
    void apply(int l, int r, F f){
        if(l >= r) return;
        l += n; r += n;
        push_down(l); push_down(r-1);
        int l0 = l, r0 = r;
        while(l < r){
            if(l & 1){ dat[l] = mapping(f, dat[l]); if(l < n) lz[l] = composition(f, lz[l]); l++; }
            if(r & 1){ r--; dat[r] = mapping(f, dat[r]); if(r < n) lz[r] = composition(f, lz[r]); }
            l >>= 1; r >>= 1;
        }
        push_up(l0); push_up(r0-1);
    }

    // --- query(l, r) ---
    // Σa[l..r) mod MOD
    S query(int l, int r){
        if(l >= r) return e();
        l += n; r += n;
        push_down(l); push_down(r-1);
        S vl = e(), vr = e();
        while(l < r){
            if(l & 1) vl = op(vl, dat[l++]);
            if(r & 1) vr = op(dat[--r], vr);
            l >>= 1; r >>= 1;
        }
        return op(vl, vr);
    }

    // --- set(p, x) ---
    void set(int p, S x){
        p += n; push_down(p); dat[p] = x; push_up(p);
    }

    // --- get(p) ---
    S get(int p){
        p += n; push_down(p); return dat[p];
    }
};

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    int N = 5;
    // a = {1, 2, 3, 4, 5}
    vector<S> init(N);
    for(int i = 0; i < N; i++) init[i] = {i + 1, 1};
    LazySegTree seg(init);

    // ----- query(l, r) -----
    cout << seg.query(0, 5).sum << "\n"; // 1+2+3+4+5 = 15
    cout << seg.query(1, 4).sum << "\n"; // 2+3+4 = 9

    // ----- apply: 区間加算 {1, v} -----
    // a[i] = 1*a[i] + 10 → a[i] += 10
    seg.apply(0, 3, {1, 10});
    // a = {11, 12, 13, 4, 5}
    cout << seg.query(0, 5).sum << "\n"; // 45

    // ----- apply: 区間代入 {0, v} -----
    // a[i] = 0*a[i] + 7 → a[i] = 7
    seg.apply(1, 4, {0, 7});
    // a = {11, 7, 7, 7, 5}
    cout << seg.query(0, 5).sum << "\n"; // 37

    // ----- apply: 区間乗算 {v, 0} -----
    // a[i] = 3*a[i] + 0 → a[i] *= 3
    seg.apply(0, 5, {3, 0});
    // a = {33, 21, 21, 21, 15}
    cout << seg.query(0, 5).sum << "\n"; // 111

    // ----- apply: 一般のアフィン {a, b} -----
    // a[i] = 2*a[i] + 1
    seg.apply(3, 5, {2, 1});
    // a = {33, 21, 21, 43, 31}
    cout << seg.query(0, 5).sum << "\n"; // 149

    // ----- set(p, x) -----
    seg.set(2, {100, 1});
    // a = {33, 21, 100, 43, 31}
    cout << seg.query(0, 5).sum << "\n"; // 228

    // ----- get(p) -----
    cout << seg.get(0).sum << "\n"; // 33
    cout << seg.get(2).sum << "\n"; // 100
    cout << seg.get(4).sum << "\n"; // 31
}
