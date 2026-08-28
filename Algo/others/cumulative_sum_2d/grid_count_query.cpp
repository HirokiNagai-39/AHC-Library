// グリッド上の矩形カウントクエリ
// H*Wグリッドに点がN個。矩形[r1,c1]-[r2,c2]内の点数をO(1)で応答。
// 計算量: O(H*W + Q)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int H, W, N, Q;
    cin >> H >> W >> N;
    vector<vector<int>> grid(H + 1, vector<int>(W + 1, 0));
    for (int i = 0; i < N; i++) {
        int r, c;
        cin >> r >> c;
        grid[r][c]++;
    }

    // 2D累積和
    vector<vector<int>> cum(H + 2, vector<int>(W + 2, 0));
    for (int i = 1; i <= H; i++)
        for (int j = 1; j <= W; j++)
            cum[i][j] = grid[i][j] + cum[i-1][j] + cum[i][j-1] - cum[i-1][j-1];

    cin >> Q;
    while (Q--) {
        int r1, c1, r2, c2;
        cin >> r1 >> c1 >> r2 >> c2;
        int ans = cum[r2][c2] - cum[r1-1][c2] - cum[r2][c1-1] + cum[r1-1][c1-1];
        cout << ans << "\n";
    }
    return 0;
}
