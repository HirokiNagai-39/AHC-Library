// 重み付き区間スケジューリング
// 各区間に重みがあり、重複しない区間集合の重みの最大値。
// DP + 二分探索。dp[i] = i番目まで考慮したときの最大重み。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<int> s(N), e(N), w(N);
    for (int i = 0; i < N; i++) cin >> s[i] >> e[i] >> w[i];

    // 終了時刻でソート
    vector<int> idx(N);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) { return e[a] < e[b]; });

    vector<int> ends(N);
    for (int i = 0; i < N; i++) ends[i] = e[idx[i]];

    // dp[i] = idx[0..i]まで考慮した最大重み
    vector<long long> dp(N + 1, 0);
    for (int i = 0; i < N; i++) {
        int j = idx[i];
        // s[j]以下の終了時刻を持つ最後の区間を二分探索
        int prev = upper_bound(ends.begin(), ends.begin() + i, s[j]) - ends.begin();
        dp[i + 1] = max(dp[i], dp[prev] + w[j]);
    }
    cout << dp[N] << endl;
    return 0;
}
