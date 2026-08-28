// セグメントツリー: 一点更新 + 区間 (最大値, 個数)
// 「最大値は何で、それが何個あるか」を O(log N) で求める
// 応用: ABC276 E 等, 最頻値の管理
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

struct S {
    long long mx;
    int cnt; // mx の個数
};
S op(S a, S b){
    if(a.mx > b.mx) return a;
    if(a.mx < b.mx) return b;
    return {a.mx, a.cnt + b.cnt}; // 同値ならカウント合算
}
S e(){ return {-1e18, 0}; }

struct SegTree {
    int n;
    vector<S> dat;
    SegTree(int n_) : n(1) { while(n < n_) n *= 2; dat.assign(2*n, e()); }
    SegTree(const vector<long long>& v) : SegTree((int)v.size()){
        for(int i = 0; i < (int)v.size(); i++) dat[n+i] = {v[i], 1};
        for(int i = n-1; i >= 1; i--) dat[i] = op(dat[2*i], dat[2*i+1]);
    }
    void set(int p, long long x){
        p += n; dat[p] = {x, 1};
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

// === 例題 ===
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
            auto res = seg.query(l, r);
            cout << "max=" << res.mx << " count=" << res.cnt << "\n";
        }
    }
}
