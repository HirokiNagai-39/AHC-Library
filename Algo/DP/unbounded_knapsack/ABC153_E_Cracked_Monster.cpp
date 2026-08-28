// ABC153 E - Crested Ibis vs Monster
// https://atcoder.jp/contests/abc153/tasks/abc153_e
// N spells, each with damage a[i] and cost b[i]. Unlimited use.
// Reduce monster HP H to 0. Minimize total MP cost. Unbounded knapsack. O(NH)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int H,N; cin>>H>>N;
    vector<int> a(N),b(N);
    for(int i=0;i<N;i++) cin>>a[i]>>b[i];
    const int INF=1e9;
    // dp[j] = min cost to deal at least j damage
    vector<int> dp(H+1,INF);
    dp[0]=0;
    for(int j=1;j<=H;j++){
        for(int i=0;i<N;i++){
            int prev=max(0,j-a[i]);
            dp[j]=min(dp[j],dp[prev]+b[i]);
        }
    }
    cout<<dp[H]<<endl;
}
