// SCC + DAG DP (longest path on condensation graph)
// SCC縮約後のDAGでDP(最長路等)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,M;cin>>N>>M;
    vector<vector<int>>g(N),rg(N);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);rg[b].push_back(a);}
    // Kosaraju SCC
    vector<int>order,comp(N,-1);vector<bool>vis(N);
    function<void(int)>dfs=[&](int v){vis[v]=true;for(int u:g[v])if(!vis[u])dfs(u);order.push_back(v);};
    for(int i=0;i<N;i++)if(!vis[i])dfs(i);
    int K=0;
    function<void(int,int)>rdfs=[&](int v,int c){comp[v]=c;for(int u:rg[v])if(comp[u]<0)rdfs(u,c);};
    for(int i=N-1;i>=0;i--)if(comp[order[i]]<0)rdfs(order[i],K++);
    // build DAG
    vector<int>sz(K,0);
    for(int i=0;i<N;i++)sz[comp[i]]++;
    vector<set<int>>dag_s(K);
    for(int v=0;v<N;v++)for(int u:g[v])if(comp[v]!=comp[u])dag_s[comp[v]].insert(comp[u]);
    vector<vector<int>>dag(K);vector<int>deg(K,0);
    for(int i=0;i<K;i++)for(int j:dag_s[i]){dag[i].push_back(j);deg[j]++;}
    // topological DP: longest path (by vertex count)
    vector<int>dp(K);
    for(int i=0;i<K;i++)dp[i]=sz[i];
    queue<int>q;for(int i=0;i<K;i++)if(deg[i]==0)q.push(i);
    int ans=0;
    while(!q.empty()){
        int v=q.front();q.pop();ans=max(ans,dp[v]);
        for(int u:dag[v]){dp[u]=max(dp[u],dp[v]+sz[u]);if(--deg[u]==0)q.push(u);}
    }
    cout<<ans<<endl;
}
