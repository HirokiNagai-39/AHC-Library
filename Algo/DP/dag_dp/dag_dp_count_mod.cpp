// DAG Path Count (mod)
// Count number of s-t paths on a DAG modulo MOD. O(V+E)
// Process in topological order, dp[v] = number of paths from s to v.
#include <bits/stdc++.h>
using namespace std;
const long long MOD=1e9+7;
int main(){
    int V,E,S,T; cin>>V>>E>>S>>T;
    vector<vector<int>> adj(V);
    vector<int> indeg(V,0);
    for(int i=0;i<E;i++){
        int u,v; cin>>u>>v;
        adj[u].push_back(v);
        indeg[v]++;
    }
    // topological sort
    queue<int> q;
    for(int i=0;i<V;i++) if(indeg[i]==0) q.push(i);
    vector<int> topo;
    while(!q.empty()){
        int v=q.front(); q.pop();
        topo.push_back(v);
        for(int u:adj[v]) if(--indeg[u]==0) q.push(u);
    }
    vector<long long> dp(V,0);
    dp[S]=1;
    for(int v:topo){
        if(dp[v]==0) continue;
        for(int u:adj[v])
            dp[u]=(dp[u]+dp[v])%MOD;
    }
    cout<<dp[T]<<endl;
}
