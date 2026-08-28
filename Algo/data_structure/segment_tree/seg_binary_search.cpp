// セグメントツリー上の二分探索
// 条件を満たす最左/最右の位置を O(log N) で求める
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

struct SegTree {
    int n;
    vector<int> d;
    SegTree(int n_) : n(1) { while(n < n_) n *= 2; d.assign(2*n, 0); }

    // a[i] = x
    void set(int i, int x){
        i += n; d[i] = x;
        while(i > 1){ i /= 2; d[i] = max(d[2*i], d[2*i+1]); }
    }

    // max(a[l..r))
    int query(int l, int r){
        int res = 0;
        for(l += n, r += n; l < r; l /= 2, r /= 2){
            if(l & 1) res = max(res, d[l++]);
            if(r & 1) res = max(res, d[--r]);
        }
        return res;
    }

    // --- find_first(x) ---
    // 全体 [0, n) から d[i] >= x を満たす最小の i を返す
    // 存在しなければ -1
    int find_first(int x){
        return find_first_impl(1, 0, n, x);
    }
    int find_first_impl(int k, int kl, int kr, int x){
        if(d[k] < x) return -1;       // この部分木には条件を満たす要素なし
        if(kr - kl == 1) return kl;    // 葉に到達
        int mid = (kl + kr) / 2;
        int res = find_first_impl(2*k, kl, mid, x);  // 左を先に探す
        if(res != -1) return res;
        return find_first_impl(2*k+1, mid, kr, x);    // 左になければ右
    }

    // --- find_first_in(l, r, x) ---
    // 区間 [l, r) 内で d[i] >= x を満たす最小の i を返す
    // 存在しなければ -1
    int find_first_in(int l, int r, int x){
        return find_first_in_impl(1, 0, n, l, r, x);
    }
    int find_first_in_impl(int k, int kl, int kr, int ql, int qr, int x){
        if(kr <= ql || qr <= kl || d[k] < x) return -1;
        if(kr - kl == 1) return kl;
        int mid = (kl + kr) / 2;
        int res = find_first_in_impl(2*k, kl, mid, ql, qr, x);
        if(res != -1) return res;
        return find_first_in_impl(2*k+1, mid, kr, ql, qr, x);
    }

    // --- find_last_in(l, r, x) ---
    // 区間 [l, r) 内で d[i] >= x を満たす最大の i を返す
    // 存在しなければ -1
    int find_last_in(int l, int r, int x){
        return find_last_in_impl(1, 0, n, l, r, x);
    }
    int find_last_in_impl(int k, int kl, int kr, int ql, int qr, int x){
        if(kr <= ql || qr <= kl || d[k] < x) return -1;
        if(kr - kl == 1) return kl;
        int mid = (kl + kr) / 2;
        int res = find_last_in_impl(2*k+1, mid, kr, ql, qr, x); // 右を先に探す
        if(res != -1) return res;
        return find_last_in_impl(2*k, kl, mid, ql, qr, x);
    }
};

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    // a = {2, 5, 1, 8, 3, 7, 4, 6}
    //      0  1  2  3  4  5  6  7
    int N = 8;
    SegTree seg(N);
    int a[] = {2, 5, 1, 8, 3, 7, 4, 6};
    for(int i = 0; i < N; i++) seg.set(i, a[i]);

    // ----- set(i, x) -----
    // a[i] = x
    seg.set(2, 10);  // a = {2, 5, 10, 8, 3, 7, 4, 6}

    // ----- query(l, r) -----
    // max(a[l..r))
    cout << seg.query(0, 8) << "\n"; // max全体 = 10
    cout << seg.query(3, 6) << "\n"; // max(8,3,7) = 8

    seg.set(2, 1);   // 元に戻す: a = {2, 5, 1, 8, 3, 7, 4, 6}

    // ----- find_first(x) -----
    // 全体から d[i] >= x の最小 i
    cout << seg.find_first(7) << "\n";  // 3 (a[3]=8 が最初の >=7)
    cout << seg.find_first(1) << "\n";  // 0 (a[0]=2 >= 1)
    cout << seg.find_first(9) << "\n";  // -1 (>=9 は存在しない)

    // ----- find_first_in(l, r, x) -----
    // [l, r) 内で d[i] >= x の最小 i
    cout << seg.find_first_in(4, 8, 5) << "\n"; // 5 (a[5]=7 が [4,8) で最初の >=5)
    cout << seg.find_first_in(0, 3, 4) << "\n";  // 1 (a[1]=5 >= 4)
    cout << seg.find_first_in(0, 3, 6) << "\n";  // -1 ([0,3) に >=6 はない)

    // ----- find_last_in(l, r, x) -----
    // [l, r) 内で d[i] >= x の最大 i
    cout << seg.find_last_in(0, 8, 5) << "\n";  // 7 (a[7]=6 が最後の >=5)
    cout << seg.find_last_in(0, 4, 3) << "\n";   // 3 (a[3]=8 が [0,4) で最後の >=3)

    return 0;
}
