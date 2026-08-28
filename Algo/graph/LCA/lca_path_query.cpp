// LCA + path query (max edge weight on path)
// パスクエリ(パス上の最大辺重み)。LCA+ダブリングで辺情報も持つ
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
static constexpr int LOG=20;
int par[100005][LOG],dep[100005];
ll mx[100005][LOG]; // max edge weight on path to 2^k ancestor
vector<pair<int,ll>>g[100005];
int N;
void dfs(int v,int p,int d){
    par[v][0]=p;dep[v]=d;
    for(auto[u,w]:g[v])if(u!=p){mx[u][0]=w;dfs(u,v,d+1);}
}
void build(){
    for(int k=0;k+1<LOG;k++)for(int v=0;v<N;v++){
        par[v][k+1]=par[v][k]<0?-1:par[par[v][k]][k];
        mx[v][k+1]=par[v][k]<0?0:max(mx[v][k],mx[par[v][k]][k]);
    }
}
pair<int,ll>lca_max(int u,int v){
    ll res=0;
    if(dep[u]<dep[v])swap(u,v);
    for(int k=LOG-1;k>=0;k--)if(dep[u]-(1<<k)>=dep[v]){res=max(res,mx[u][k]);u=par[u][k];}
    if(u==v)return{u,res};
    for(int k=LOG-1;k>=0;k--)if(par[u][k]!=par[v][k]){res=max({res,mx[u][k],mx[v][k]});u=par[u][k];v=par[v][k];}
    res=max({res,mx[u][0],mx[v][0]});
    return{par[u][0],res};
}
int main(){
    int Q;cin>>N>>Q;
    for(int i=0;i<N-1;i++){int a,b;ll c;cin>>a>>b>>c;a--;b--;g[a].push_back({b,c});g[b].push_back({a,c});}
    memset(par,-1,sizeof(par));memset(mx,0,sizeof(mx));
    dfs(0,-1,0);build();
    while(Q--){int u,v;cin>>u>>v;u--;v--;auto[l,m]=lca_max(u,v);cout<<m<<"\n";}
}
