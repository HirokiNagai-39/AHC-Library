// LCA: weighted tree distance query
// 重み付き木のパス上の距離。LCA+累積重み
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
static constexpr int LOG=20;
int par[100005][LOG],dep[100005];
ll wdist[100005]; // distance from root
vector<pair<int,ll>>g[100005];
int N;
void dfs(int v,int p,int d,ll w){par[v][0]=p;dep[v]=d;wdist[v]=w;
    for(auto[u,c]:g[v])if(u!=p)dfs(u,v,d+1,w+c);}
void build(){for(int k=0;k+1<LOG;k++)for(int v=0;v<N;v++)par[v][k+1]=par[v][k]<0?-1:par[par[v][k]][k];}
int lca(int u,int v){
    if(dep[u]<dep[v])swap(u,v);
    for(int k=LOG-1;k>=0;k--)if(dep[u]-(1<<k)>=dep[v])u=par[u][k];
    if(u==v)return u;
    for(int k=LOG-1;k>=0;k--)if(par[u][k]!=par[v][k]){u=par[u][k];v=par[v][k];}
    return par[u][0];
}
ll dist(int u,int v){return wdist[u]+wdist[v]-2*wdist[lca(u,v)];}
int main(){
    int Q;cin>>N>>Q;
    for(int i=0;i<N-1;i++){int a,b;ll c;cin>>a>>b>>c;a--;b--;g[a].push_back({b,c});g[b].push_back({a,c});}
    dfs(0,-1,0,0);build();
    while(Q--){int u,v;cin>>u>>v;u--;v--;cout<<dist(u,v)<<"\n";}
}
