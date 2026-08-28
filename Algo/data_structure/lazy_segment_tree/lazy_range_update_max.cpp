// 遅延セグメントツリー: 区間代入 + 区間最大値
// apply(l, r, v): a[i] = v  for i in [l, r)
// query(l, r): max(a[l..r))
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static constexpr ll NEG_INF = -1e18;
static constexpr ll NONE = -2e18;

struct LazySegTree {
    int n;
    vector<ll> d, lz;

    LazySegTree(int n_) : n(1) {
        while(n < n_) n <<= 1;
        d.assign(2*n, NEG_INF); lz.assign(2*n, NONE);
    }
    LazySegTree(const vector<ll>& v) : LazySegTree((int)v.size()) {
        for(int i = 0; i < (int)v.size(); i++) d[n+i] = v[i];
        for(int i = n-1; i >= 1; i--) d[i] = max(d[2*i], d[2*i+1]);
    }

    void push(int k){
        if(lz[k] == NONE) return;
        d[2*k] = lz[k]; lz[2*k] = lz[k];
        d[2*k+1] = lz[k]; lz[2*k+1] = lz[k];
        lz[k] = NONE;
    }

    // --- apply(l, r, v) ---
    // a[i] = v  for i in [l, r)
    void apply(int l, int r, ll v){ apply_impl(1, 0, n, l, r, v); }
    void apply_impl(int k, int kl, int kr, int l, int r, ll v){
        if(kr <= l || r <= kl) return;
        if(l <= kl && kr <= r){ d[k] = v; lz[k] = v; return; }
        push(k);
        int mid = (kl+kr)/2;
        apply_impl(2*k, kl, mid, l, r, v);
        apply_impl(2*k+1, mid, kr, l, r, v);
        d[k] = max(d[2*k], d[2*k+1]);
    }

    // --- query(l, r) ---
    // max(a[l..r))
    ll query(int l, int r){ return query_impl(1, 0, n, l, r); }
    ll query_impl(int k, int kl, int kr, int l, int r){
        if(kr <= l || r <= kl) return NEG_INF;
        if(l <= kl && kr <= r) return d[k];
        push(k);
        int mid = (kl+kr)/2;
        return max(query_impl(2*k, kl, mid, l, r),
                   query_impl(2*k+1, mid, kr, l, r));
    }
};

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    vector<ll> a = {2, 7, 1, 8, 3};
    LazySegTree seg(a);

    // ----- query(l, r) -----
    cout << seg.query(0, 5) << "\n"; // max(2,7,1,8,3) = 8
    cout << seg.query(0, 2) << "\n"; // max(2,7) = 7
    cout << seg.query(2, 5) << "\n"; // max(1,8,3) = 8

    // ----- apply(l, r, v) -----
    seg.apply(2, 5, 5);
    // a = {2, 7, 5, 5, 5}
    cout << seg.query(0, 5) << "\n"; // max = 7
    cout << seg.query(2, 5) << "\n"; // max(5,5,5) = 5

    seg.apply(0, 5, 100);
    // a = {100, 100, 100, 100, 100}
    cout << seg.query(0, 5) << "\n"; // 100
}
