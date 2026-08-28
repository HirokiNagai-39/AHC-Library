// LCA: distance query on tree
// 木上の2点間距離クエリ。dist(u,v)=depth[u]+depth[v]-2*depth[lca(u,v)]
#include <bits/stdc++.h>
using namespace std;
static constexpr int LOG=20;
int par[100005][LOG],dep[100005];
vector<int>g[100005];
int N;
void dfs(int v,int p,int d){par[v][0]=p;dep[v]=d;for(int u:g[v])if(u!=p)dfs(u,v,d+1);}
void build(){for(int k=0;k+1<LOG;k++)for(int v=0;v<N;v++)par[v][k+1]=par[v][k]<0?-1:par[par[v][k]][k];}
int lca(int u,int v){
    if(dep[u]<dep[v])swap(u,v);
    for(int k=LOG-1;k>=0;k--)if(dep[u]-(1<<k)>=dep[v])u=par[u][k];
    if(u==v)return u;
    for(int k=LOG-1;k>=0;k--)if(par[u][k]!=par[v][k]){u=par[u][k];v=par[v][k];}
    return par[u][0];
}
int dist(int u,int v){return dep[u]+dep[v]-2*dep[lca(u,v)];}
int main(){
    int Q;cin>>N>>Q;
    for(int i=0;i<N-1;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);g[b].push_back(a);}
    dfs(0,-1,0);build();
    while(Q--){int u,v;cin>>u>>v;u--;v--;cout<<dist(u,v)<<"\n";}
}
