// EDPC Q - Flowers
// https://atcoder.jp/contests/dp/tasks/dp_q
// N flowers with heights h[i] and beauties a[i]. Remove some so remaining
// heights are increasing. Maximize total beauty. Weighted LIS with segment tree. O(NlogN)
#include <bits/stdc++.h>
using namespace std;

struct SegTree {
    int n;
    vector<long long> tree;
    SegTree(int n) : n(n), tree(4 * n, 0) {}
    void update(int pos, long long val, int node, int lo, int hi){
        if(lo == hi){ tree[node] = max(tree[node], val); return; }
        int mid = (lo + hi) / 2;
        if(pos <= mid) update(pos, val, 2*node, lo, mid);
        else update(pos, val, 2*node+1, mid+1, hi);
        tree[node] = max(tree[2*node], tree[2*node+1]);
    }
    void update(int pos, long long val){ update(pos, val, 1, 0, n-1); }
    long long query(int l, int r, int node, int lo, int hi){
        if(r < lo || hi < l) return 0;
        if(l <= lo && hi <= r) return tree[node];
        int mid = (lo + hi) / 2;
        return max(query(l, r, 2*node, lo, mid), query(l, r, 2*node+1, mid+1, hi));
    }
    long long query(int l, int r){ return (l > r) ? 0 : query(l, r, 1, 0, n-1); }
};

int main(){
    int N;
    cin >> N;
    vector<int> h(N);
    vector<long long> a(N);
    for(auto &x : h) cin >> x;
    for(auto &x : a) cin >> x;

    SegTree seg(N + 1);
    long long ans = 0;
    for(int i = 0; i < N; i++){
        long long best = seg.query(0, h[i] - 1) + a[i];
        ans = max(ans, best);
        seg.update(h[i], best);
    }
    cout << ans << endl;
}
