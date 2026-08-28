// ABC203 D - Pond
// https://atcoder.jp/contests/abc203/tasks/abc203_d
// N*Nグリッド中のK*K部分行列の中央値の最小値。
// 答えで二分探索 + 2D累積和で閾値以下の個数をカウント。
// 計算量: O(N^2 log max(A))
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, K;
    cin >> N >> K;
    vector<vector<int>> A(N, vector<int>(N));
    for (auto& row : A) for (auto& a : row) cin >> a;

    int lo = 0, hi = 1e9;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        // 各K*K部分で mid 以下が K*K/2+1 個以上あるか
        vector<vector<int>> cum(N + 1, vector<int>(N + 1, 0));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                cum[i+1][j+1] = (A[i][j] <= mid ? 1 : 0)
                    + cum[i][j+1] + cum[i+1][j] - cum[i][j];

        bool ok = false;
        int need = (K * K + 1) / 2; // 中央値がmid以下 ⇔ mid以下がneed個以上
        for (int i = K; i <= N && !ok; i++)
            for (int j = K; j <= N && !ok; j++) {
                int cnt = cum[i][j] - cum[i-K][j] - cum[i][j-K] + cum[i-K][j-K];
                if (cnt >= need) ok = true;
            }
        if (ok) hi = mid;
        else lo = mid + 1;
    }
    cout << lo << endl;
    return 0;
}
