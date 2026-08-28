// 最短ハミルトン閉路 (対称コスト版TSP)
// 完全グラフで全頂点を巡回する最小コスト
// 計算量: O(N^2 * 2^N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<vector<int>> d(N,vector<int>(N));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) cin>>d[i][j];
    const int INF=1e9;
    vector<vector<int>> dp(1<<N,vector<int>(N,INF));
    dp[1][0]=0;
    for(int S=1;S<(1<<N);S++) for(int v=0;v<N;v++){
        if(dp[S][v]>=INF||!(S>>v&1)) continue;
        for(int u=0;u<N;u++) if(!(S>>u&1))
            dp[S|(1<<u)][u]=min(dp[S|(1<<u)][u],dp[S][v]+d[v][u]);
    }
    int ans=INF;
    for(int v=0;v<N;v++) ans=min(ans,dp[(1<<N)-1][v]+d[v][0]);
    cout<<ans<<endl;
}
