// ABC205 D - Kth Excluded
// https://atcoder.jp/contests/abc205/tasks/abc205_d
// 集合Aに含まれないK番目の正整数。二分探索。
// x以下でAに含まれない数の個数 = x - (A中でx以下の個数)
// 計算量: O((N+Q) log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, Q;
    cin >> N >> Q;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;
    sort(A.begin(), A.end());

    while (Q--) {
        long long K;
        cin >> K;
        // x - (A中でx以下の個数) >= K となる最小x
        long long lo = 0, hi = 2e18;
        while (hi - lo > 1) {
            long long mid = lo + (hi - lo) / 2;
            long long cnt = mid - (upper_bound(A.begin(), A.end(), mid) - A.begin());
            if (cnt >= K) hi = mid;
            else lo = mid;
        }
        cout << hi << "\n";
    }
    return 0;
}
