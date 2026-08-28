// ABC106 D - AtCoder Express 2
// https://atcoder.jp/contests/abc106/tasks/abc106_d
// 区間[L,R]に収まる列車数。2D累積和で O(1) クエリ。
// 計算量: O(N^2 + M + Q)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M, Q;
    cin >> N >> M >> Q;

    // cnt[l][r] = [l,r]区間の列車数
    vector<vector<int>> cnt(N + 2, vector<int>(N + 2, 0));
    for (int i = 0; i < M; i++) {
        int l, r;
        cin >> l >> r;
        cnt[l][r]++;
    }

    // 2D累積和: sum[i][j] = sum of cnt[a][b] for 1<=a<=i, 1<=b<=j
    vector<vector<int>> sum(N + 2, vector<int>(N + 2, 0));
    for (int i = 1; i <= N; i++)
        for (int j = 1; j <= N; j++)
            sum[i][j] = cnt[i][j] + sum[i-1][j] + sum[i][j-1] - sum[i-1][j-1];

    while (Q--) {
        int l, r;
        cin >> l >> r;
        // 1<=a<=l かつ b<=r => sum[l][r] - sum[l-1 doesn't work directly]
        // l<=a, b<=r: sum[l..N][1..r] minus sum[l..N][1..l-1]
        // Actually: want sum_{a>=l, b<=r} cnt[a][b]
        // = sum[N][r] - sum[l-1][r] (partial) ... need careful 2D
        cout << sum[r][r] - sum[l-1][r] - sum[r][l-1] + sum[l-1][l-1] << "\n";
    }
    return 0;
}
