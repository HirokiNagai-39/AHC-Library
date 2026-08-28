// 最大長方形和 (Maximum Subarray in 2D)
// 2D累積和+カダネのアルゴリズムで最大部分長方形和。
// 計算量: O(H^2 * W)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int H, W;
    cin >> H >> W;
    vector<vector<long long>> A(H, vector<long long>(W));
    for (auto& row : A) for (auto& a : row) cin >> a;

    long long ans = LLONG_MIN;
    // 上端r1, 下端r2を固定し、列方向に1Dカダネ
    for (int r1 = 0; r1 < H; r1++) {
        vector<long long> col_sum(W, 0);
        for (int r2 = r1; r2 < H; r2++) {
            for (int j = 0; j < W; j++) col_sum[j] += A[r2][j];
            // 1Dカダネ
            long long cur = 0;
            for (int j = 0; j < W; j++) {
                cur = max(col_sum[j], cur + col_sum[j]);
                ans = max(ans, cur);
            }
        }
    }
    cout << ans << endl;
    return 0;
}
