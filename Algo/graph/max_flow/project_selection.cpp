// Project selection (burn or bury / source-sink modeling)
// プロジェクト選択問題(燃やす埋める)テンプレート
// Minimize: sum of penalties. Model as min-cut.
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct MaxFlow{
    struct E{int to,rev;ll cap;};
    vector<vector<E>>g;vector<int>level,iter;
    MaxFlow(int n):g(n),level(n),iter(n){}
    void add(int from,int to,ll cap){
        g[from].push_back({to,(int)g[to].size(),cap});
        g[to].push_back({from,(int)g[from].size()-1,0});
    }
    bool bfs(int s,int t){fill(level.begin(),level.end(),-1);queue<int>q;level[s]=0;q.push(s);
        while(!q.empty()){int v=q.front();q.pop();for(auto&e:g[v])if(e.cap>0&&level[e.to]<0){level[e.to]=level[v]+1;q.push(e.to);}}
        return level[t]>=0;}
    ll dfs(int v,int t,ll f){if(v==t)return f;for(int&i=iter[v];i<(int)g[v].size();i++){
        auto&e=g[v][i];if(e.cap>0&&level[v]<level[e.to]){ll d=dfs(e.to,t,min(f,e.cap));
        if(d>0){e.cap-=d;g[e.to][e.rev].cap+=d;return d;}}}return 0;}
    ll max_flow(int s,int t){ll flow=0;while(bfs(s,t)){fill(iter.begin(),iter.end(),0);ll d;while((d=dfs(s,t,1e18))>0)flow+=d;}return flow;}
};
int main(){
    // Example: N items, each can be in group S or T
    // profit[i] for choosing S, penalty for (i in S, j in T) pairs
    int N,M;cin>>N>>M;
    int S=N,T=N+1;
    MaxFlow mf(N+2);
    ll total=0;
    for(int i=0;i<N;i++){ll a,b;cin>>a>>b; // a=profit for S, b=profit for T
        total+=a+b;mf.add(S,i,a);mf.add(i,T,b);}
    for(int i=0;i<M;i++){int u,v;ll w;cin>>u>>v>>w;u--;v--;
        mf.add(u,v,w);} // penalty w if u in S and v in T
    cout<<total-mf.max_flow(S,T)<<endl;
}
