// 巡回セールスマン問題 (非対称コスト)
// 例題: ABC180 E - Traveling Salesman among Aerial Cities
// https://atcoder.jp/contests/abc180/tasks/abc180_e
// 計算量: O(N^2 * 2^N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<long long> X(N),Y(N),Z(N);
    for(int i=0;i<N;i++) cin>>X[i]>>Y[i]>>Z[i];
    vector<vector<long long>> c(N,vector<long long>(N));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++)
        c[i][j]=abs(X[i]-X[j])+abs(Y[i]-Y[j])+max(0LL,Z[j]-Z[i]);
    const long long INF=1e18;
    vector<vector<long long>> dp(1<<N,vector<long long>(N,INF));
    dp[1][0]=0;
    for(int S=1;S<(1<<N);S++) for(int v=0;v<N;v++){
        if(dp[S][v]>=INF) continue;
        for(int u=0;u<N;u++) if(!(S>>u&1))
            dp[S|(1<<u)][u]=min(dp[S|(1<<u)][u],dp[S][v]+c[v][u]);
    }
    long long ans=INF;
    for(int v=0;v<N;v++) ans=min(ans,dp[(1<<N)-1][v]+c[v][0]);
    cout<<ans<<endl;
}
