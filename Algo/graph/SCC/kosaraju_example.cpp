// Kosaraju's algorithm: basic example
// 到達可能ペア数。SCC内は全ペア到達可能、DAG上の到達も加算
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
int main(){
    int N,M;cin>>N>>M;
    vector<vector<int>>g(N),rg(N);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);rg[b].push_back(a);}
    vector<int>order,comp(N,-1);vector<bool>vis(N);
    function<void(int)>dfs=[&](int v){vis[v]=true;for(int u:g[v])if(!vis[u])dfs(u);order.push_back(v);};
    for(int i=0;i<N;i++)if(!vis[i])dfs(i);
    int K=0;
    function<void(int,int)>rdfs=[&](int v,int c){comp[v]=c;for(int u:rg[v])if(comp[u]<0)rdfs(u,c);};
    for(int i=N-1;i>=0;i--)if(comp[order[i]]<0)rdfs(order[i],K++);
    vector<ll>sz(K,0);
    for(int i=0;i<N;i++)sz[comp[i]]++;
    // DAG reachable set via DP (topological order = 0..K-1 already)
    vector<set<int>>reach(K);
    vector<vector<int>>dag(K);
    for(int v=0;v<N;v++)for(int u:g[v])if(comp[v]!=comp[u])dag[comp[v]].insert(comp[u]);
    // count reachable vertices from each SCC
    // simple: for each SCC, BFS on DAG
    ll ans=0;
    for(int s=0;s<K;s++){
        vector<bool>vis2(K);queue<int>q;vis2[s]=true;q.push(s);ll cnt=0;
        while(!q.empty()){int v=q.front();q.pop();cnt+=sz[v];for(int u:dag[v])if(!vis2[u]){vis2[u]=true;q.push(u);}}
        ans+=sz[s]*cnt; // each vertex in s can reach cnt vertices
    }
    cout<<ans<<endl;
}
