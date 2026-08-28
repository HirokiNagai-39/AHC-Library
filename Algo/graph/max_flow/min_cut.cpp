// Min cut = Max flow (by max-flow min-cut theorem)
// 最小カット。BFS残余グラフでS側頂点を求める
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
    vector<bool> min_cut_side(int s){// returns S-side vertices after max_flow
        int n=g.size();vector<bool>vis(n);queue<int>q;vis[s]=true;q.push(s);
        while(!q.empty()){int v=q.front();q.pop();for(auto&e:g[v])if(e.cap>0&&!vis[e.to]){vis[e.to]=true;q.push(e.to);}}
        return vis;}
};
int main(){
    int N,M;cin>>N>>M;
    int S=0,T=N-1;
    MaxFlow mf(N);
    for(int i=0;i<M;i++){int a,b;ll c;cin>>a>>b>>c;a--;b--;mf.add(a,b,c);}
    ll flow=mf.max_flow(S,T);
    cout<<flow<<"\n";
    auto side=mf.min_cut_side(S);
    cout<<"S-side:";for(int i=0;i<N;i++)if(side[i])cout<<" "<<i+1;cout<<endl;
}
