// 2Dいもす法
// H*Wグリッドに矩形加算を複数回行い、最後に全体をbuild。
// imos[r1][c1]++, imos[r1][c2+1]--, imos[r2+1][c1]--, imos[r2+1][c2+1]++
// 計算量: O(H*W + Q)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int H, W, Q;
    cin >> H >> W >> Q;
    vector<vector<long long>> imos(H + 2, vector<long long>(W + 2, 0));

    while (Q--) {
        int r1, c1, r2, c2;
        long long v;
        cin >> r1 >> c1 >> r2 >> c2 >> v;
        imos[r1][c1] += v;
        imos[r1][c2 + 1] -= v;
        imos[r2 + 1][c1] -= v;
        imos[r2 + 1][c2 + 1] += v;
    }

    // build: 行方向 → 列方向に累積和
    for (int i = 1; i <= H; i++)
        for (int j = 1; j <= W; j++)
            imos[i][j] += imos[i-1][j] + imos[i][j-1] - imos[i-1][j-1];

    for (int i = 1; i <= H; i++) {
        for (int j = 1; j <= W; j++)
            cout << imos[i][j] << " \n"[j == W];
    }
    return 0;
}
