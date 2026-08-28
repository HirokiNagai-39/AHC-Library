// LCA via Euler Tour + Sparse Table (RMQ)
// オイラーツアー+RMQによるLCA。O(NlogN)前処理、O(1)クエリ
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,Q;cin>>N>>Q;
    vector<vector<int>>g(N);
    for(int i=0;i<N-1;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);g[b].push_back(a);}
    // Euler tour
    vector<int>euler,depth(N),first(N),dep_euler;
    function<void(int,int,int)>dfs=[&](int v,int p,int d){
        depth[v]=d;first[v]=euler.size();euler.push_back(v);dep_euler.push_back(d);
        for(int u:g[v])if(u!=p){dfs(u,v,d+1);euler.push_back(v);dep_euler.push_back(d);}
    };
    dfs(0,-1,0);
    int M=euler.size(),LOG=__lg(M)+1;
    // sparse table on dep_euler (index of minimum)
    vector<vector<int>>sp(LOG,vector<int>(M));
    iota(sp[0].begin(),sp[0].end(),0);
    for(int k=1;k<LOG;k++)for(int i=0;i+(1<<k)<=M;i++){
        int a=sp[k-1][i],b=sp[k-1][i+(1<<(k-1))];
        sp[k][i]=dep_euler[a]<=dep_euler[b]?a:b;
    }
    auto query=[&](int l,int r)->int{int k=__lg(r-l+1);
        int a=sp[k][l],b=sp[k][r-(1<<k)+1];return dep_euler[a]<=dep_euler[b]?a:b;};
    while(Q--){
        int u,v;cin>>u>>v;u--;v--;
        int l=first[u],r=first[v];if(l>r)swap(l,r);
        cout<<euler[query(l,r)]+1<<"\n";
    }
}
