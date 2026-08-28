// Bipartite matching via max flow
// 二部マッチング最大数。Dinic's algorithm
#include <bits/stdc++.h>
using namespace std;
struct MaxFlow{
    struct E{int to,rev;int cap;};
    vector<vector<E>>g;vector<int>level,iter;
    MaxFlow(int n):g(n),level(n),iter(n){}
    void add(int from,int to,int cap){
        g[from].push_back({to,(int)g[to].size(),cap});
        g[to].push_back({from,(int)g[from].size()-1,0});
    }
    bool bfs(int s,int t){fill(level.begin(),level.end(),-1);queue<int>q;level[s]=0;q.push(s);
        while(!q.empty()){int v=q.front();q.pop();for(auto&e:g[v])if(e.cap>0&&level[e.to]<0){level[e.to]=level[v]+1;q.push(e.to);}}
        return level[t]>=0;}
    int dfs(int v,int t,int f){if(v==t)return f;for(int&i=iter[v];i<(int)g[v].size();i++){
        auto&e=g[v][i];if(e.cap>0&&level[v]<level[e.to]){int d=dfs(e.to,t,min(f,e.cap));
        if(d>0){e.cap-=d;g[e.to][e.rev].cap+=d;return d;}}}return 0;}
    int max_flow(int s,int t){int flow=0;while(bfs(s,t)){fill(iter.begin(),iter.end(),0);int d;while((d=dfs(s,t,INT_MAX))>0)flow+=d;}return flow;}
};
int main(){
    int L,R,M;cin>>L>>R>>M; // L left nodes, R right nodes
    int S=L+R,T=S+1;
    MaxFlow mf(T+1);
    for(int i=0;i<L;i++)mf.add(S,i,1);
    for(int i=0;i<R;i++)mf.add(L+i,T,1);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b;a--;b--;mf.add(a,L+b,1);}
    cout<<mf.max_flow(S,T)<<endl;
}
