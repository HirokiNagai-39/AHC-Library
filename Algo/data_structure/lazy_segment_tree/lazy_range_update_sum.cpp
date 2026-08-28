// 遅延セグメントツリー: 区間代入 + 区間和
// apply(l, r, v): a[i] = v  for i in [l, r)  (全て v に置き換え)
// query(l, r): Σa[l..r)
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static constexpr ll NONE = -1e18;

struct LazySegTree {
    int n;
    vector<ll> sum_;
    vector<int> sz;
    vector<ll> lz;

    LazySegTree(int n_) : n(1) {
        while(n < n_) n *= 2;
        sum_.assign(2*n, 0); sz.assign(2*n, 0); lz.assign(2*n, NONE);
        for(int i = 0; i < n; i++) sz[n+i] = 1;
        for(int i = n-1; i >= 1; i--) sz[i] = sz[2*i] + sz[2*i+1];
    }
    LazySegTree(const vector<ll>& v) : LazySegTree((int)v.size()) {
        for(int i = 0; i < (int)v.size(); i++) sum_[n+i] = v[i];
        for(int i = n-1; i >= 1; i--) sum_[i] = sum_[2*i] + sum_[2*i+1];
    }

    void push(int k){
        if(lz[k] == NONE) return;
        for(int c : {2*k, 2*k+1}){
            sum_[c] = lz[k] * sz[c];
            lz[c] = lz[k];
        }
        lz[k] = NONE;
    }

    // --- apply(l, r, v) ---
    // a[i] = v  for i in [l, r)  (区間代入)
    void apply(int l, int r, ll v){ apply_impl(1, 0, n, l, r, v); }
    void apply_impl(int k, int kl, int kr, int l, int r, ll v){
        if(kr <= l || r <= kl) return;
        if(l <= kl && kr <= r){ sum_[k] = v * sz[k]; lz[k] = v; return; }
        push(k);
        int mid = (kl+kr)/2;
        apply_impl(2*k, kl, mid, l, r, v);
        apply_impl(2*k+1, mid, kr, l, r, v);
        sum_[k] = sum_[2*k] + sum_[2*k+1];
    }

    // --- query(l, r) ---
    // Σa[l..r)  (区間和)
    ll query(int l, int r){ return query_impl(1, 0, n, l, r); }
    ll query_impl(int k, int kl, int kr, int l, int r){
        if(kr <= l || r <= kl) return 0;
        if(l <= kl && kr <= r) return sum_[k];
        push(k);
        int mid = (kl+kr)/2;
        return query_impl(2*k, kl, mid, l, r) +
               query_impl(2*k+1, mid, kr, l, r);
    }
};

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    vector<ll> a = {1, 2, 3, 4, 5};
    LazySegTree seg(a);

    // ----- query(l, r) -----
    cout << seg.query(0, 5) << "\n"; // 1+2+3+4+5 = 15
    cout << seg.query(1, 4) << "\n"; // 2+3+4 = 9
    cout << seg.query(0, 1) << "\n"; // 1

    // ----- apply(l, r, v) -----
    seg.apply(1, 4, 10);
    // a = {1, 10, 10, 10, 5}
    cout << seg.query(0, 5) << "\n"; // 36
    cout << seg.query(1, 4) << "\n"; // 30

    seg.apply(0, 5, 0);
    // a = {0, 0, 0, 0, 0}
    cout << seg.query(0, 5) << "\n"; // 0

    seg.apply(2, 4, 100);
    // a = {0, 0, 100, 100, 0}
    cout << seg.query(0, 5) << "\n"; // 200

    // 重ね掛け: 後の apply が上書き
    seg.apply(0, 3, 5);
    // a = {5, 5, 5, 100, 0}
    cout << seg.query(0, 5) << "\n"; // 115
}
