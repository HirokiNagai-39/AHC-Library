// Bellman-Ford: Shortest path with negative edges
// 負辺を含む最短路の基本。SSSP。N-1回緩和
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
const ll INF=1e18;
int main(){
    int N,M,S;cin>>N>>M>>S;S--;
    struct E{int a,b;ll w;};
    vector<E>edges(M);
    for(auto&[a,b,w]:edges){cin>>a>>b>>w;a--;b--;}
    vector<ll>d(N,INF);d[S]=0;
    for(int i=0;i<N-1;i++)
        for(auto&[a,b,w]:edges)
            if(d[a]<INF)d[b]=min(d[b],d[a]+w);
    // check negative cycle
    bool neg=false;
    for(auto&[a,b,w]:edges)
        if(d[a]<INF&&d[a]+w<d[b]){neg=true;break;}
    if(neg){cout<<"NEGATIVE CYCLE"<<endl;return 0;}
    for(int i=0;i<N;i++){
        if(d[i]>=INF)cout<<"INF"<<"\n";
        else cout<<d[i]<<"\n";
    }
}
