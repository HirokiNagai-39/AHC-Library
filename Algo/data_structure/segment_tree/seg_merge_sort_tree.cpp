// マージソートツリー
// 各ノードにソート済み配列を持つセグメントツリー
// クエリ: 区間内の値の個数 / k番目に小さい値
// 計算量: 構築 O(N log N), クエリ O(log^2 N) or O(log^3 N)
// ※ 一点更新は不可 (静的データ構造)
#include <bits/stdc++.h>
using namespace std;

struct MergeSortTree {
    int n;
    vector<vector<int>> dat;

    // --- コンストラクタ(配列から構築) ---
    MergeSortTree(const vector<int>& a){
        int sz = a.size();
        n = 1;
        while(n < sz) n *= 2;
        dat.resize(2 * n);
        for(int i = 0; i < sz; i++) dat[n + i] = {a[i]};
        for(int i = n - 1; i >= 1; i--){
            merge(dat[2*i].begin(), dat[2*i].end(),
                  dat[2*i+1].begin(), dat[2*i+1].end(),
                  back_inserter(dat[i]));
        }
    }

    // --- count_le(l, r, x) ---
    // [l, r) で x 以下の要素数  (≤ x)
    int count_le(int l, int r, int x){
        int res = 0;
        for(l += n, r += n; l < r; l >>= 1, r >>= 1){
            if(l & 1){ res += upper_bound(dat[l].begin(), dat[l].end(), x) - dat[l].begin(); l++; }
            if(r & 1){ r--; res += upper_bound(dat[r].begin(), dat[r].end(), x) - dat[r].begin(); }
        }
        return res;
    }

    // --- count_lt(l, r, x) ---
    // [l, r) で x 未満の要素数  (< x)
    int count_lt(int l, int r, int x){
        int res = 0;
        for(l += n, r += n; l < r; l >>= 1, r >>= 1){
            if(l & 1){ res += lower_bound(dat[l].begin(), dat[l].end(), x) - dat[l].begin(); l++; }
            if(r & 1){ r--; res += lower_bound(dat[r].begin(), dat[r].end(), x) - dat[r].begin(); }
        }
        return res;
    }

    // --- count_range(l, r, lo, hi) ---
    // [l, r) で lo 以上 hi 以下の要素数  (lo ≤ val ≤ hi)
    int count_range(int l, int r, int lo, int hi){
        return count_le(l, r, hi) - count_lt(l, r, lo);
    }

    // --- kth_smallest(l, r, k) ---
    // [l, r) で k番目 (1-indexed) に小さい値
    // 二分探索: count_le(l, r, mid) >= k を満たす最小の mid
    int kth_smallest(int l, int r, int k){
        int lo = -1e9, hi = 1e9;
        while(hi - lo > 1){
            int mid = lo + (hi - lo) / 2;
            if(count_le(l, r, mid) >= k) hi = mid;
            else lo = mid;
        }
        return hi;
    }
};

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    // a = {5, 1, 3, 8, 2, 7, 4, 6}
    //      0  1  2  3  4  5  6  7
    vector<int> a = {5, 1, 3, 8, 2, 7, 4, 6};
    MergeSortTree mst(a);

    // ----- count_le(l, r, x) -----
    // [l, r) で x 以下の要素数
    cout << mst.count_le(0, 8, 4) << "\n"; // {1,2,3,4} の4個 (≤4)
    cout << mst.count_le(0, 4, 4) << "\n"; // [0,4)={5,1,3,8} で ≤4 → {1,3} = 2個
    cout << mst.count_le(2, 6, 5) << "\n"; // [2,6)={3,8,2,7} で ≤5 → {3,2} = 2個

    // ----- count_lt(l, r, x) -----
    // [l, r) で x 未満の要素数
    cout << mst.count_lt(0, 8, 4) << "\n"; // {1,2,3} の3個 (<4)
    cout << mst.count_lt(0, 8, 1) << "\n"; // 0個 (<1 はない)

    // ----- count_range(l, r, lo, hi) -----
    // [l, r) で lo ≤ val ≤ hi の要素数
    cout << mst.count_range(0, 8, 3, 6) << "\n"; // {5,3,4,6} = 4個 (3≤val≤6)
    cout << mst.count_range(2, 7, 2, 4) << "\n";  // [2,7)={3,8,2,7,4} で 2≤val≤4 → {3,2,4} = 3個

    // ----- kth_smallest(l, r, k) -----
    // [l, r) で k番目に小さい値 (1-indexed)
    cout << mst.kth_smallest(0, 8, 1) << "\n"; // 全体で1番目 = 1
    cout << mst.kth_smallest(0, 8, 4) << "\n"; // 全体で4番目 = 4
    cout << mst.kth_smallest(0, 8, 8) << "\n"; // 全体で8番目 = 8
    cout << mst.kth_smallest(2, 6, 2) << "\n"; // [2,6)={3,8,2,7} で2番目 = 3

    return 0;
}
