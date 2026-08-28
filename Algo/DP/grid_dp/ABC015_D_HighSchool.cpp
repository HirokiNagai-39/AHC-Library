// ABC015 D - High School
// https://atcoder.jp/contests/abc015/tasks/abc015_d
// Width W, N items, pick at most K items with total width <= W, maximize value.
// 0-1 knapsack with item count constraint. O(NWK)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int W, N, K;
    cin >> W >> N >> K;
    vector<int> a(N), b(N);
    for(int i = 0; i < N; i++) cin >> a[i] >> b[i];

    // dp[j][k] = max value using weight j and k items
    vector<vector<long long>> dp(W + 1, vector<long long>(K + 1, -1));
    dp[0][0] = 0;

    for(int i = 0; i < N; i++){
        for(int j = W; j >= a[i]; j--){
            for(int k = K; k >= 1; k--){
                if(dp[j - a[i]][k - 1] < 0) continue;
                dp[j][k] = max(dp[j][k], dp[j - a[i]][k - 1] + b[i]);
            }
        }
    }

    long long ans = 0;
    for(int j = 0; j <= W; j++)
        for(int k = 0; k <= K; k++)
            if(dp[j][k] >= 0) ans = max(ans, dp[j][k]);
    cout << ans << endl;
}
