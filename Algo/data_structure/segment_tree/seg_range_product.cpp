// セグメントツリー: 一点更新 + 区間積 (mod)
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 998244353;

long long op_mul(long long a, long long b){ return a % MOD * (b % MOD) % MOD; }
long long e_mul(){ return 1; } // 乗法の単位元

template<typename T, T (*op)(T, T), T (*e)()>
struct SegTree {
    int n;
    vector<T> dat;
    SegTree(int n_) : n(1) { while(n < n_) n *= 2; dat.assign(2*n, e()); }
    SegTree(const vector<T>& v) : SegTree((int)v.size()){
        for(int i = 0; i < (int)v.size(); i++) dat[n+i] = v[i];
        for(int i = n-1; i >= 1; i--) dat[i] = op(dat[2*i], dat[2*i+1]);
    }
    void set(int p, T x){
        p += n; dat[p] = x;
        while(p >>= 1) dat[p] = op(dat[2*p], dat[2*p+1]);
    }
    T query(int l, int r){
        T vl = e(), vr = e();
        for(l += n, r += n; l < r; l >>= 1, r >>= 1){
            if(l & 1) vl = op(vl, dat[l++]);
            if(r & 1) vr = op(dat[--r], vr);
        }
        return op(vl, vr);
    }
};

// === 例題: 区間積 mod p ===
int main(){
    int N, Q;
    cin >> N >> Q;
    vector<long long> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];
    SegTree<long long, op_mul, e_mul> seg(a);
    while(Q--){
        int t; cin >> t;
        if(t == 1){
            int i; long long x; cin >> i >> x;
            seg.set(i, x % MOD);
        } else {
            int l, r; cin >> l >> r;
            cout << seg.query(l, r) << "\n";
        }
    }
}
