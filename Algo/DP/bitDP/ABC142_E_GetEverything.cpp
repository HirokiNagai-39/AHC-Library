// ABC142 E - Get Everything
// https://atcoder.jp/contests/abc142/tasks/abc142_e
// N boxes with locks, M key bundles. Find min cost to open all boxes.
// dp[S] = min cost to open set S. O(M * 2^N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,M; cin>>N>>M;
    vector<int> cost(M),mask(M);
    for(int i=0;i<M;i++){
        int b; cin>>cost[i]>>b;
        for(int j=0;j<b;j++){int c;cin>>c;c--;mask[i]|=(1<<c);}
    }
    const int INF=1e9;
    vector<int> dp(1<<N,INF);
    dp[0]=0;
    for(int S=0;S<(1<<N);S++){
        if(dp[S]==INF) continue;
        for(int i=0;i<M;i++){
            int nS=S|mask[i];
            dp[nS]=min(dp[nS],dp[S]+cost[i]);
        }
    }
    cout<<(dp[(1<<N)-1]==INF?-1:dp[(1<<N)-1])<<endl;
}
