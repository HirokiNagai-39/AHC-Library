// 遅延セグメントツリー: 区間代入 + 区間最小値
// apply(l, r, v): a[i] = v  for i in [l, r)
// query(l, r): min(a[l..r))
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static constexpr ll INF = 1e18;
static constexpr ll NONE = -1e18;

struct LazySegTree {
    int n;
    vector<ll> d, lz;

    LazySegTree(int n_) : n(1) {
        while(n < n_) n <<= 1;
        d.assign(2*n, INF); lz.assign(2*n, NONE);
    }
    LazySegTree(const vector<ll>& v) : LazySegTree((int)v.size()) {
        for(int i = 0; i < (int)v.size(); i++) d[n+i] = v[i];
        for(int i = n-1; i >= 1; i--) d[i] = min(d[2*i], d[2*i+1]);
    }

    void push(int k){
        if(lz[k] == NONE) return;
        d[2*k] = lz[k]; lz[2*k] = lz[k];
        d[2*k+1] = lz[k]; lz[2*k+1] = lz[k];
        lz[k] = NONE;
    }

    // --- apply(l, r, v) ---
    // a[i] = v  for i in [l, r)  (区間代入)
    void apply(int l, int r, ll v){ apply_impl(1, 0, n, l, r, v); }
    void apply_impl(int k, int kl, int kr, int l, int r, ll v){
        if(kr <= l || r <= kl) return;
        if(l <= kl && kr <= r){ d[k] = v; lz[k] = v; return; }
        push(k);
        int mid = (kl+kr)/2;
        apply_impl(2*k, kl, mid, l, r, v);
        apply_impl(2*k+1, mid, kr, l, r, v);
        d[k] = min(d[2*k], d[2*k+1]);
    }

    // --- query(l, r) ---
    // min(a[l..r))
    ll query(int l, int r){ return query_impl(1, 0, n, l, r); }
    ll query_impl(int k, int kl, int kr, int l, int r){
        if(kr <= l || r <= kl) return INF;
        if(l <= kl && kr <= r) return d[k];
        push(k);
        int mid = (kl+kr)/2;
        return min(query_impl(2*k, kl, mid, l, r),
                   query_impl(2*k+1, mid, kr, l, r));
    }
};

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    vector<ll> a = {5, 3, 8, 1, 6};
    LazySegTree seg(a);

    // ----- query(l, r) -----
    cout << seg.query(0, 5) << "\n"; // min(5,3,8,1,6) = 1
    cout << seg.query(0, 3) << "\n"; // min(5,3,8) = 3
    cout << seg.query(2, 4) << "\n"; // min(8,1) = 1

    // ----- apply(l, r, v) -----
    // [l, r) を全て v に代入
    seg.apply(0, 3, 10);
    // a = {10, 10, 10, 1, 6}
    cout << seg.query(0, 5) << "\n"; // min = 1
    cout << seg.query(0, 3) << "\n"; // min(10,10,10) = 10

    seg.apply(3, 5, 20);
    // a = {10, 10, 10, 20, 20}
    cout << seg.query(0, 5) << "\n"; // min = 10

    // 上書き: 後の apply が優先
    seg.apply(1, 4, 2);
    // a = {10, 2, 2, 2, 20}
    cout << seg.query(0, 5) << "\n"; // min = 2
    cout << seg.query(0, 1) << "\n"; // 10
    cout << seg.query(4, 5) << "\n"; // 20
}
