// Rerooting DP - Maximum Distance from Each Vertex
// For each vertex, find the farthest vertex distance. O(N)
// Equivalent to computing tree diameter endpoints' distances efficiently.
// dp1[v] = max dist going down in subtree, dp2[v] = max dist going up through parent.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<vector<pair<int,long long>>> adj(N);
    for(int i=0;i<N-1;i++){
        int u,v; long long w;
        cin>>u>>v>>w; u--;v--;
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
    }
    vector<int> par(N,-1);
    vector<int> order;
    vector<bool> visited(N,false);
    {
        queue<int> q; q.push(0); visited[0]=true;
        while(!q.empty()){
            int v=q.front(); q.pop(); order.push_back(v);
            for(auto [u,w]:adj[v]) if(!visited[u]){visited[u]=true;par[u]=v;q.push(u);}
        }
    }
    // dp_down[v] = max distance going down from v
    // Also store top1, top2 (best and second best child paths)
    vector<long long> dp_down(N,0);
    vector<long long> top1(N,0),top2(N,0); // best, 2nd best going down
    vector<int> best_child(N,-1);
    for(int i=N-1;i>=0;i--){
        int v=order[i];
        for(auto [u,w]:adj[v]){
            if(u==par[v]) continue;
            long long val=dp_down[u]+w;
            if(val>=top1[v]){ top2[v]=top1[v]; top1[v]=val; best_child[v]=u; }
            else if(val>top2[v]) top2[v]=val;
        }
        dp_down[v]=top1[v];
    }
    // dp_up[v] = max distance going up from v through parent
    vector<long long> dp_up(N,0);
    for(int v:order){
        for(auto [u,w]:adj[v]){
            if(u==par[v]) continue;
            // up path for u: go to v, then either go up from v, or go down from v through another child
            long long from_v_up=dp_up[v]+w;
            long long from_v_down=(best_child[v]==u?top2[v]:top1[v])+w;
            dp_up[u]=max(from_v_up,from_v_down);
        }
    }
    for(int i=0;i<N;i++)
        cout<<max(dp_down[i],dp_up[i])<<"\n";
}
