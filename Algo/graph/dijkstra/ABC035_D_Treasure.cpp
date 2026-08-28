// ABC035 D - Treasure
// https://atcoder.jp/contests/abc035/tasks/abc035_d
// 往復最短路。正逆グラフでDijkstra2回。O((V+E)logV)
#include <bits/stdc++.h>
using namespace std;
using P=pair<long long,int>;
vector<long long> dijkstra(const vector<vector<P>>&g,int s){
    int n=g.size();vector<long long>d(n,1e18);
    priority_queue<P,vector<P>,greater<P>>pq;d[s]=0;pq.push({0,s});
    while(!pq.empty()){auto[c,v]=pq.top();pq.pop();if(c>d[v])continue;
    for(auto[w,u]:g[v])if(d[v]+w<d[u]){d[u]=d[v]+w;pq.push({d[u],u});}}return d;}
int main(){
    int N,M,T;cin>>N>>M>>T;
    vector<long long>A(N);for(auto&a:A)cin>>a;
    vector<vector<P>>g(N),rg(N);
    for(int i=0;i<M;i++){int a,b;long long c;cin>>a>>b>>c;a--;b--;g[a].push_back({c,b});rg[b].push_back({c,a});}
    auto d1=dijkstra(g,0),d2=dijkstra(rg,0);
    long long ans=0;
    for(int i=0;i<N;i++)if(d1[i]<1e18&&d2[i]<1e18)ans=max(ans,A[i]*(T-d1[i]-d2[i]));
    cout<<ans<<endl;
}
