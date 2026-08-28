// セグメントツリー: 一点更新 + 区間最大値
// 例題: ABC133 F 等
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

struct SegTree {
    int n;
    vector<long long> d;
    static constexpr long long NEG_INF = -1e18;
    SegTree(int n_) : n(1) { while(n < n_) n *= 2; d.assign(2*n, NEG_INF); }
    // a[i] = x
    void set(int i, long long x){
        i += n; d[i] = x;
        while(i > 1){ i /= 2; d[i] = max(d[2*i], d[2*i+1]); }
    }
    // max(a[l..r))
    long long query(int l, int r){
        long long res = NEG_INF;
        for(l += n, r += n; l < r; l /= 2, r /= 2){
            if(l & 1) res = max(res, d[l++]);
            if(r & 1) res = max(res, d[--r]);
        }
        return res;
    }
};

// === 例題: 動的に値を更新しながら区間最大値を求める ===
int main(){
    int N, Q;
    cin >> N >> Q;
    SegTree seg(N);
    for(int i = 0; i < N; i++){ long long x; cin >> x; seg.set(i, x); }
    while(Q--){
        int t; cin >> t;
        if(t == 1){ int i; long long x; cin >> i >> x; seg.set(i, x); }
        else { int l, r; cin >> l >> r; cout << seg.query(l, r) << "\n"; }
    }
}
