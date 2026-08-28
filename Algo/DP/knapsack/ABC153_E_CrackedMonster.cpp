// ABC153 E - Crested Ibis vs Monster
// https://atcoder.jp/contests/abc153/tasks/abc153_e
// Minimize total magic cost to reduce monster HP to 0. Unbounded knapsack variant. O(NH)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int H, N;
    cin >> H >> N;
    vector<int> a(N), b(N);
    for(int i = 0; i < N; i++) cin >> a[i] >> b[i];

    // dp[j] = min cost to deal exactly j or more damage
    static constexpr long long INF = 1e18;
    vector<long long> dp(H + 1, INF);
    dp[0] = 0;

    for(int j = 1; j <= H; j++){
        for(int i = 0; i < N; i++){
            int prev = max(0, j - a[i]);
            dp[j] = min(dp[j], dp[prev] + b[i]);
        }
    }
    cout << dp[H] << endl;
}
