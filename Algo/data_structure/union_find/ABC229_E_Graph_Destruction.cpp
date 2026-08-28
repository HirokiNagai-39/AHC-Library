// ABC229 E - Graph Destruction
// https://atcoder.jp/contests/abc229/tasks/abc229_e
// 頂点逆順追加でUF。頂点Nから1へ追加し連結成分数を記録
#include <bits/stdc++.h>
using namespace std;
struct UF{
    vector<int>p,sz;int comp;
    UF(int n):p(n),sz(n,1),comp(0){iota(p.begin(),p.end(),0);}
    int find(int x){return p[x]==x?x:p[x]=find(p[x]);}
    bool unite(int a,int b){a=find(a);b=find(b);if(a==b)return false;
        if(sz[a]<sz[b])swap(a,b);p[b]=a;sz[a]+=sz[b];comp--;return true;}
};
int main(){
    int N,M;cin>>N>>M;
    vector<vector<int>>g(N);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);g[b].push_back(a);}
    UF uf(N);
    vector<bool>active(N,false);
    vector<int>ans(N);
    // add vertices from N-1 down to 0
    for(int v=N-1;v>=0;v--){
        active[v]=true;uf.comp++;
        for(int u:g[v])if(active[u])uf.unite(v,u);
        ans[v]=uf.comp;
    }
    // answer for removing vertex 1..N: after removing v, answer = state before adding v
    for(int i=0;i<N;i++)cout<<(i+1<N?ans[i+1]:0)<<"\n";
}
