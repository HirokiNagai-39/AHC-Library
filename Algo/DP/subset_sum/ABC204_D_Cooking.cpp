// ABC204 D - Cooking
// https://atcoder.jp/contests/abc204/tasks/abc204_d
// N dishes, each takes T[i] time. Two ovens run in parallel.
// Minimize total time = max(sum_oven1, sum_oven2).
// Equivalent: subset sum to minimize max(X, S-X). O(N * sum(T))
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<int> T(N);
    int S = 0;
    for(auto &t : T){ cin >> t; S += t; }

    // dp[j] = can we assign subset with total time j to oven 1?
    vector<bool> dp(S + 1, false);
    dp[0] = true;
    for(int i = 0; i < N; i++)
        for(int j = S; j >= T[i]; j--)
            dp[j] = dp[j] || dp[j - T[i]];

    int ans = S;
    for(int j = 0; j <= S; j++)
        if(dp[j]) ans = min(ans, max(j, S - j));
    cout << ans << endl;
}
