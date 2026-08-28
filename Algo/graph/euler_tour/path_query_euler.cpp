// Euler Tour + Segment Tree: path sum query (vertex values)
// パスクエリをETで処理。ET(in/out)+BIT差分で頂点加算・パス和
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct BIT{
    int n;vector<ll>d;BIT(){}BIT(int n):n(n),d(n+1){}
    void add(int i,ll x){for(i++;i<=n;i+=i&-i)d[i]+=x;}
    ll sum(int i){ll s=0;for(;i>0;i-=i&-i)s+=d[i];return s;}
};
int par_[100005],dep[100005],in_[100005],out_[100005];
vector<int>g[100005];
int N,timer_=0;
BIT b1,b2;
void dfs(int v,int p,int d){par_[v]=p;dep[v]=d;in_[v]=timer_++;for(int u:g[v])if(u!=p)dfs(u,v,d+1);out_[v]=timer_;}
// add x to vertex v -> add to in_[v], subtract at out_[v]
// path sum root to v = BIT prefix sum at in_[v]
void vertex_add(int v,ll x){b1.add(in_[v],x);b1.add(out_[v],-x);}
ll path_sum_to_root(int v){return b1.sum(in_[v]+1);}
int main(){
    static constexpr int LOG=17;
    int Q;cin>>N>>Q;
    for(int i=0;i<N-1;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);g[b].push_back(a);}
    vector<ll>val(N);for(auto&v:val)cin>>v;
    dfs(0,-1,0);
    // LCA for path query
    int anc[100005][17]={};
    for(int i=0;i<N;i++)anc[i][0]=par_[i];anc[0][0]=0;
    for(int k=1;k<LOG;k++)for(int i=0;i<N;i++)anc[i][k]=anc[anc[i][k-1]][k-1];
    auto lca=[&](int u,int v){
        if(dep[u]<dep[v])swap(u,v);
        for(int k=LOG-1;k>=0;k--)if(dep[u]-(1<<k)>=dep[v])u=anc[u][k];
        if(u==v)return u;
        for(int k=LOG-1;k>=0;k--)if(anc[u][k]!=anc[v][k]){u=anc[u][k];v=anc[v][k];}
        return anc[u][0];
    };
    b1=BIT(2*N);
    for(int i=0;i<N;i++)vertex_add(i,val[i]);
    while(Q--){
        int t;cin>>t;
        if(t==1){int v;ll x;cin>>v>>x;v--;vertex_add(v,x-val[v]);val[v]=x;}
        else{int u,v;cin>>u>>v;u--;v--;int l=lca(u,v);
            cout<<path_sum_to_root(u)+path_sum_to_root(v)-path_sum_to_root(l)-(l?path_sum_to_root(anc[l][0]):0)+val[l]<<"\n";
            // correction: subtract lca counted twice, add back val[lca] once
        }
    }
}
