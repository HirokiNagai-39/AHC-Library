// Euler Tour + BIT: subtree update + point query
// 部分木一括加算+頂点値取得。ET+BIT(imos法)
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct BIT{
    int n;vector<ll>d;BIT(int n):n(n),d(n+2){}
    void add(int i,ll x){for(i++;i<=n;i+=i&-i)d[i]+=x;}
    void add(int l,int r,ll x){add(l,x);add(r,-x);} // [l,r)
    ll query(int i){ll s=0;for(i++;i>0;i-=i&-i)s+=d[i];return s;} // prefix sum = point value
};
int main(){
    int N,Q;cin>>N>>Q;
    vector<vector<int>>g(N);
    for(int i=0;i<N-1;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);g[b].push_back(a);}
    vector<int>in(N),out(N);int timer=0;
    function<void(int,int)>dfs=[&](int v,int p){in[v]=timer++;for(int u:g[v])if(u!=p)dfs(u,v);out[v]=timer;};
    dfs(0,-1);
    BIT bit(N);
    while(Q--){
        int t;cin>>t;
        if(t==1){int v;ll x;cin>>v>>x;v--;bit.add(in[v],out[v],x);} // add x to subtree of v
        else{int v;cin>>v;v--;cout<<bit.query(in[v])<<"\n";} // get value of v
    }
}
