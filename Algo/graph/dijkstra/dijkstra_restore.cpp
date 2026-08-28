// Dijkstra with path restoration
// prev配列で最短路を復元して出力
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
using P=pair<ll,int>;
int main(){
    int N,M,S,T;cin>>N>>M>>S>>T;S--;T--;
    vector<vector<P>>g(N);
    for(int i=0;i<M;i++){int a,b;ll c;cin>>a>>b>>c;a--;b--;g[a].push_back({c,b});g[b].push_back({c,a});}
    vector<ll>d(N,1e18);vector<int>prev(N,-1);
    priority_queue<P,vector<P>,greater<P>>pq;
    d[S]=0;pq.push({0,S});
    while(!pq.empty()){
        auto[c,v]=pq.top();pq.pop();
        if(c>d[v])continue;
        for(auto[w,u]:g[v])if(d[v]+w<d[u]){d[u]=d[v]+w;prev[u]=v;pq.push({d[u],u});}
    }
    if(d[T]>=1e18){cout<<-1<<endl;return 0;}
    vector<int>path;for(int v=T;v!=-1;v=prev[v])path.push_back(v);
    reverse(path.begin(),path.end());
    cout<<d[T]<<"\n";
    for(int i=0;i<(int)path.size();i++)cout<<path[i]+1<<" \n"[i+1==(int)path.size()];
}
