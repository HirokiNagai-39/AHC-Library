// ABC143 E - Travel by Car
// https://atcoder.jp/contests/abc143/tasks/abc143_e
// 燃料L制限での最小給油回数。WF2回
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
const ll INF=1e18;
int main(){
    int N,M;ll L;cin>>N>>M>>L;
    vector<vector<ll>>d(N,vector<ll>(N,INF));
    for(int i=0;i<N;i++)d[i][i]=0;
    for(int i=0;i<M;i++){int a,b;ll c;cin>>a>>b>>c;a--;b--;d[a][b]=d[b][a]=min(d[a][b],c);}
    // WF for shortest distance
    for(int k=0;k<N;k++)for(int i=0;i<N;i++)for(int j=0;j<N;j++)
        if(d[i][k]<INF&&d[k][j]<INF)d[i][j]=min(d[i][j],d[i][k]+d[k][j]);
    // build refuel graph: edge exists if dist <= L
    vector<vector<ll>>f(N,vector<ll>(N,INF));
    for(int i=0;i<N;i++){f[i][i]=0;for(int j=0;j<N;j++)if(d[i][j]<=L)f[i][j]=1;}
    // WF for minimum refuels
    for(int k=0;k<N;k++)for(int i=0;i<N;i++)for(int j=0;j<N;j++)
        if(f[i][k]<INF&&f[k][j]<INF)f[i][j]=min(f[i][j],f[i][k]+f[k][j]);
    int Q;cin>>Q;
    while(Q--){int s,t;cin>>s>>t;s--;t--;cout<<(f[s][t]<INF?f[s][t]-1:-1)<<"\n";}
}
