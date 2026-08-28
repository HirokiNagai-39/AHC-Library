// Euler Tour + BIT: subtree sum query
// 部分木の値の和クエリ。ET+BIT
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct BIT{
    int n;vector<ll>d;BIT(int n):n(n),d(n+1){}
    void add(int i,ll x){for(i++;i<=n;i+=i&-i)d[i]+=x;}
    ll sum(int i){ll s=0;for(;i>0;i-=i&-i)s+=d[i];return s;}
    ll sum(int l,int r){return sum(r)-sum(l);} // [l,r)
};
int main(){
    int N,Q;cin>>N>>Q;
    vector<vector<int>>g(N);
    for(int i=0;i<N-1;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);g[b].push_back(a);}
    vector<ll>val(N);for(auto&v:val)cin>>v;
    vector<int>in(N),out(N);int timer=0;
    function<void(int,int)>dfs=[&](int v,int p){in[v]=timer++;for(int u:g[v])if(u!=p)dfs(u,v);out[v]=timer;};
    dfs(0,-1);
    BIT bit(N);
    for(int i=0;i<N;i++)bit.add(in[i],val[i]);
    while(Q--){
        int t;cin>>t;
        if(t==1){int v;ll x;cin>>v>>x;v--;bit.add(in[v],x-val[v]);val[v]=x;}
        else{int v;cin>>v;v--;cout<<bit.sum(in[v],out[v])<<"\n";}
    }
}
