// 差分配列 (いもす法の1D版)
// 区間[l,r]にvを加算する操作をO(1)で行い、最後に累積和で復元。
// 計算量: O(N + Q)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, Q;
    cin >> N >> Q;
    vector<long long> diff(N + 1, 0);

    while (Q--) {
        int l, r;
        long long v;
        cin >> l >> r >> v; // 0-indexed [l, r]
        diff[l] += v;
        if (r + 1 <= N) diff[r + 1] -= v;
    }

    // 累積和で復元
    vector<long long> A(N);
    A[0] = diff[0];
    for (int i = 1; i < N; i++) A[i] = A[i-1] + diff[i];

    for (int i = 0; i < N; i++) cout << A[i] << " \n"[i==N-1];
    return 0;
}
