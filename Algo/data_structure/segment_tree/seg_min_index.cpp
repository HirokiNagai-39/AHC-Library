// セグメントツリー: 一点更新 + 区間 (最小値, そのインデックス)
// 区間最小値とその位置を同時に取得
// 応用: RMQ + 復元, ダイクストラの代替
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

struct S {
    long long val;
    int idx;
};
S op(S a, S b){ return a.val <= b.val ? a : b; } // 同値はインデックス小優先
S e(){ return {(long long)1e18, -1}; }

struct SegTree {
    int n;
    vector<S> dat;
    SegTree(int n_) : n(1) { while(n < n_) n *= 2; dat.assign(2*n, e()); }
    SegTree(const vector<long long>& v) : SegTree((int)v.size()){
        for(int i = 0; i < (int)v.size(); i++) dat[n+i] = {v[i], i};
        for(int i = n-1; i >= 1; i--) dat[i] = op(dat[2*i], dat[2*i+1]);
    }
    void set(int p, long long x){
        p += n; dat[p] = {x, p - n};
        while(p >>= 1) dat[p] = op(dat[2*p], dat[2*p+1]);
    }
    S query(int l, int r){
        S vl = e(), vr = e();
        for(l += n, r += n; l < r; l >>= 1, r >>= 1){
            if(l & 1) vl = op(vl, dat[l++]);
            if(r & 1) vr = op(dat[--r], vr);
        }
        return op(vl, vr);
    }
};

// === 例題: 区間最小値とその位置 ===
int main(){
    int N, Q;
    cin >> N >> Q;
    vector<long long> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];
    SegTree seg(a);
    while(Q--){
        int t; cin >> t;
        if(t == 1){
            int i; long long x; cin >> i >> x;
            seg.set(i, x);
        } else {
            int l, r; cin >> l >> r;
            auto [val, idx] = seg.query(l, r);
            cout << "min=" << val << " at index=" << idx << "\n";
        }
    }
}
