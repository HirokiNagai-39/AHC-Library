// Cumulative Sum DP Template
// Pattern: dp[i] = sum of dp[j] for j in some range [l(i), r(i)]
// Accelerated using prefix sums from O(N^2) to O(N).
// Example: dp[i] = sum(dp[j]) for j in [i-K, i-1], dp[0]=1. Count ways to reach N.
#include <bits/stdc++.h>
using namespace std;
const long long MOD=1e9+7;
int main(){
    int N,K; cin>>N>>K;
    // dp[i] = number of ways to reach position i, stepping 1..K at a time
    vector<long long> dp(N+1,0);
    dp[0]=1;
    // S[i] = dp[0]+dp[1]+...+dp[i] (prefix sum)
    vector<long long> S(N+2,0);
    S[0]=1;
    for(int i=1;i<=N;i++){
        // dp[i] = sum(dp[j]) for j in [max(0,i-K), i-1]
        int lo=max(0,i-K);
        dp[i]=(S[i-1]-(lo>0?S[lo-1]:0)+MOD)%MOD;
        S[i]=(S[i-1]+dp[i])%MOD;
    }
    cout<<dp[N]<<endl;
}
