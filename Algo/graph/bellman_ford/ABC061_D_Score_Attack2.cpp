// ABC061 D - Score Attack (alternative: Bellman-Ford)
// https://atcoder.jp/contests/abc061/tasks/abc061_d
// 辺重みを反転して最長路 → Bellman-Fordで負閉路=正閉路検出
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
const ll INF=1e18;
int main(){
    int N,M;cin>>N>>M;
    struct E{int a,b;ll w;};
    vector<E>edges(M);
    for(auto&[a,b,w]:edges){cin>>a>>b>>w;a--;b--;w=-w;} // negate for longest path
    vector<ll>d(N,INF);d[0]=0;
    for(int i=0;i<N-1;i++)
        for(auto&[a,b,w]:edges)
            if(d[a]<INF)d[b]=min(d[b],d[a]+w);
    // check if negative cycle reachable to N-1
    // run N more rounds, if d[N-1] changes => inf
    for(int i=0;i<N;i++)
        for(auto&[a,b,w]:edges)
            if(d[a]<INF&&d[a]+w<d[b]){d[b]=d[a]+w;if(b==N-1){cout<<"inf"<<endl;return 0;}}
    cout<<-d[N-1]<<endl;
}
