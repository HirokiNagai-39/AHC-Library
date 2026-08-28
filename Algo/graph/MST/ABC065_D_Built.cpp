// ABC065 D - Built? (ARC076 B)
// https://atcoder.jp/contests/abc065/tasks/arc076_b
// 2D座標のMST。x,yでソートして隣接辺のみ追加。O(NlogN)
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
    int N;cin>>N;
    vector<ll>x(N),y(N);
    for(int i=0;i<N;i++)cin>>x[i]>>y[i];
    vector<tuple<ll,int,int>>edges;
    // sort by x, add adjacent edges
    vector<int>idx(N);iota(idx.begin(),idx.end(),0);
    sort(idx.begin(),idx.end(),[&](int a,int b){return x[a]<x[b];});
    for(int i=0;i+1<N;i++)edges.push_back({abs(x[idx[i]]-x[idx[i+1]]),idx[i],idx[i+1]});
    // sort by y, add adjacent edges
    sort(idx.begin(),idx.end(),[&](int a,int b){return y[a]<y[b];});
    for(int i=0;i+1<N;i++)edges.push_back({abs(y[idx[i]]-y[idx[i+1]]),idx[i],idx[i+1]});
    sort(edges.begin(),edges.end());
    UF uf(N);ll ans=0;
    for(auto[w,a,b]:edges)if(uf.unite(a,b))ans+=w;
    cout<<ans<<endl;
}
