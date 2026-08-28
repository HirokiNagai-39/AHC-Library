// 遅延セグメントツリー: 区間加算 + 区間最小値
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static constexpr ll INF = 1e18;

struct LazySegTree {
    int n;
    vector<ll> d, lz;

    // --- コンストラクタ ---
    // (1) サイズ指定: 全要素 INF
    LazySegTree(int n_) : n(1) {
        while(n < n_) n *= 2;
        d.assign(2*n, INF); lz.assign(2*n, 0);
    }
    // (2) 配列から構築
    LazySegTree(const vector<ll>& v) : LazySegTree((int)v.size()) {
        for(int i = 0; i < (int)v.size(); i++) d[n+i] = v[i];
        for(int i = n-1; i >= 1; i--) d[i] = min(d[2*i], d[2*i+1]);
    }

    void push(int k){
        if(!lz[k]) return;
        d[2*k] += lz[k]; lz[2*k] += lz[k];
        d[2*k+1] += lz[k]; lz[2*k+1] += lz[k];
        lz[k] = 0;
    }

    // --- update(l, r, x) ---
    // a[i] += x  for i in [l, r)  (区間加算)
    void update(int l, int r, ll x){ update_impl(1, 0, n, l, r, x); }
    void update_impl(int k, int kl, int kr, int l, int r, ll x){
        if(kr <= l || r <= kl) return;
        if(l <= kl && kr <= r){ d[k] += x; lz[k] += x; return; }
        push(k);
        int mid = (kl+kr)/2;
        update_impl(2*k, kl, mid, l, r, x);
        update_impl(2*k+1, mid, kr, l, r, x);
        d[k] = min(d[2*k], d[2*k+1]);
    }

    // --- query(l, r) ---
    // min(a[l..r))  (区間最小値)
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
    // a = {5, 3, 8, 1, 6}
    vector<ll> a = {5, 3, 8, 1, 6};
    LazySegTree seg(a);

    // ----- query(l, r) -----
    // [l, r) の最小値
    cout << seg.query(0, 5) << "\n"; // min(5,3,8,1,6) = 1
    cout << seg.query(0, 3) << "\n"; // min(5,3,8) = 3
    cout << seg.query(2, 5) << "\n"; // min(8,1,6) = 1

    // ----- update(l, r, x) -----
    // [l, r) の全要素に x を加算
    seg.update(0, 3, 10);
    // a = {15, 13, 18, 1, 6}
    cout << seg.query(0, 5) << "\n"; // min = 1
    cout << seg.query(0, 3) << "\n"; // min(15,13,18) = 13

    seg.update(2, 5, -5);
    // a = {15, 13, 13, -4, 1}
    cout << seg.query(0, 5) << "\n"; // min = -4
    cout << seg.query(3, 5) << "\n"; // min(-4, 1) = -4

    // 全体加算
    seg.update(0, 5, 100);
    // a = {115, 113, 113, 96, 101}
    cout << seg.query(0, 5) << "\n"; // min = 96

    // サイズ指定コンストラクタ + 1要素ずつセット
    {
        LazySegTree seg2(5); // 全要素INF
        seg2.update(0, 1, -INF + 3); // a[0] = 3 (INF + (-INF+3) = 3)
        // → サイズ指定の場合は配列版を使うほうが安全
    }
}
