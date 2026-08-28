// ハミルトンパス (全頂点を1回ずつ通るパスの存在/最小コスト)
// TSPとの違い: 始点に戻らない
// 計算量: O(N^2 * 2^N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,M; cin>>N>>M;
    const long long INF=1e18;
    vector<vector<long long>> cost(N,vector<long long>(N,INF));
    for(int i=0;i<M;i++){
        int u,v; long long w; cin>>u>>v>>w; u--;v--;
        cost[u][v]=min(cost[u][v],w);
    }
    vector<vector<long long>> dp(1<<N,vector<long long>(N,INF));
    for(int i=0;i<N;i++) dp[1<<i][i]=0;
    for(int S=1;S<(1<<N);S++) for(int v=0;v<N;v++){
        if(dp[S][v]>=INF) continue;
        for(int u=0;u<N;u++) if(!(S>>u&1) && cost[v][u]<INF)
            dp[S|(1<<u)][u]=min(dp[S|(1<<u)][u],dp[S][v]+cost[v][u]);
    }
    long long ans=INF;
    for(int v=0;v<N;v++) ans=min(ans,dp[(1<<N)-1][v]);
    cout<<(ans>=INF?-1:ans)<<endl;
}
