// Second Minimum Spanning Tree
// 次小全域木。MSTの各辺を除去して再計算。O(EV) simple approach
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct UF{
    vector<int>p,r;UF(int n):p(n),r(n,0){iota(p.begin(),p.end(),0);}
    int find(int x){return p[x]==x?x:p[x]=find(p[x]);}
    bool unite(int a,int b){a=find(a);b=find(b);if(a==b)return false;
        if(r[a]<r[b])swap(a,b);p[b]=a;if(r[a]==r[b])r[a]++;return true;}
};
int main(){
    int N,M;cin>>N>>M;
    struct E{int a,b;ll w;};
    vector<E>edges(M);
    for(auto&[a,b,w]:edges){cin>>a>>b>>w;a--;b--;}
    vector<int>idx(M);iota(idx.begin(),idx.end(),0);
    sort(idx.begin(),idx.end(),[&](int i,int j){return edges[i].w<edges[j].w;});
    // find MST edges
    UF uf(N);vector<int>mst_edges;ll mst_cost=0;
    for(int i:idx)if(uf.unite(edges[i].a,edges[i].b)){mst_edges.push_back(i);mst_cost+=edges[i].w;}
    // try removing each MST edge
    ll ans=LLONG_MAX;
    for(int skip:mst_edges){
        UF uf2(N);ll cost=0;int cnt=0;
        for(int i:idx)if(i!=skip&&uf2.unite(edges[i].a,edges[i].b)){cost+=edges[i].w;cnt++;}
        if(cnt==N-1)ans=min(ans,cost);
    }
    if(ans==LLONG_MAX)cout<<-1<<endl;
    else cout<<ans<<endl;
}
