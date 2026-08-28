// DAG Minimum Cost Path
// Find shortest path on a DAG from source to all vertices using topological sort. O(V+E)
// Unlike Dijkstra, handles negative weights. Requires DAG structure.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int V,E,S; cin>>V>>E>>S; // vertices, edges, source
    vector<vector<pair<int,long long>>> adj(V);
    vector<int> indeg(V,0);
    for(int i=0;i<E;i++){
        int u,v; long long w;
        cin>>u>>v>>w;
        adj[u].push_back({v,w});
        indeg[v]++;
    }
    // topological sort
    queue<int> q;
    for(int i=0;i<V;i++) if(indeg[i]==0) q.push(i);
    vector<int> topo;
    while(!q.empty()){
        int v=q.front(); q.pop();
        topo.push_back(v);
        for(auto [u,w]:adj[v]) if(--indeg[u]==0) q.push(u);
    }
    const long long INF=1e18;
    vector<long long> dist(V,INF);
    dist[S]=0;
    for(int v:topo){
        if(dist[v]==INF) continue;
        for(auto [u,w]:adj[v])
            dist[u]=min(dist[u],dist[v]+w);
    }
    for(int i=0;i<V;i++){
        if(dist[i]==INF) cout<<"INF\n";
        else cout<<dist[i]<<"\n";
    }
}
