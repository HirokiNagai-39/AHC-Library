// Coin Change - Count combinations (unbounded)
// Given N coin types with values c[i], count ways to make sum S. O(NS)
// Each coin can be used unlimited times. Order doesn't matter (combinations, not permutations).
#include <bits/stdc++.h>
using namespace std;
const long long MOD=1e9+7;
int main(){
    int N,S; cin>>N>>S;
    vector<int> c(N);
    for(int i=0;i<N;i++) cin>>c[i];
    // dp[j] = number of ways to make sum j
    vector<long long> dp(S+1,0);
    dp[0]=1;
    for(int i=0;i<N;i++){
        for(int j=c[i];j<=S;j++){
            dp[j]=(dp[j]+dp[j-c[i]])%MOD;
        }
    }
    cout<<dp[S]<<endl;
}
// Note: If asking for minimum number of coins (not count), use:
// dp[j] = min(dp[j], dp[j-c[i]] + 1) with dp[0]=0, dp[j]=INF initially.
