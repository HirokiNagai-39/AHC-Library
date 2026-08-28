// ABC129 C - Stairs
// https://atcoder.jp/contests/abc129/tasks/abc129_c
// N steps staircase, some broken. Climb 1 or 2 steps at a time. Count ways. O(N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1000000007;

int main(){
    int n, m;
    cin >> n >> m;
    vector<bool> broken(n + 1, false);
    for(int i = 0; i < m; i++){
        int a; cin >> a;
        broken[a] = true;
    }

    vector<long long> dp(n + 1, 0);
    dp[0] = 1;
    for(int i = 1; i <= n; i++){
        if(broken[i]) continue;
        dp[i] = dp[i-1];
        if(i >= 2) dp[i] = (dp[i] + dp[i-2]) % MOD;
    }
    cout << dp[n] << endl;
}
