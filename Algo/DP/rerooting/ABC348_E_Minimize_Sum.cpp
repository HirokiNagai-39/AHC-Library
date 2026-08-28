// ABC348 E - Minimize Sum of Distances
// https://atcoder.jp/contests/abc348/tasks/abc348_e
// Each vertex v has weight C[v]. Find vertex minimizing sum of C[u]*dist(v,u).
// Rerooting DP. O(N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<vector<int>> adj(N);
    for(int i=0;i<N-1;i++){
        int u,v; cin>>u>>v; u--;v--;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<long long> C(N);
    for(int i=0;i<N;i++) cin>>C[i];
    // BFS order
    vector<int> par(N,-1), order;
    vector<bool> visited(N,false);
    {
        queue<int> q; q.push(0); visited[0]=true;
        while(!q.empty()){
            int v=q.front(); q.pop(); order.push_back(v);
            for(int u:adj[v]) if(!visited[u]){visited[u]=true;par[u]=v;q.push(u);}
        }
    }
    // sz[v] = sum of C in subtree of v (rooted at 0)
    vector<long long> sz(N);
    for(int i=0;i<N;i++) sz[i]=C[i];
    // dp[v] = sum of C[u]*dist(v,u) for u in subtree of v
    vector<long long> dp(N,0);
    long long totalC=0;
    for(int i=0;i<N;i++) totalC+=C[i];
    // bottom-up
    for(int i=N-1;i>=0;i--){
        int v=order[i];
        for(int u:adj[v]) if(u!=par[v]){
            sz[v]+=sz[u];
            dp[v]+=dp[u]+sz[u]; // each node in subtree(u) gets +1 dist
        }
    }
    // rerooting: ans[v] = total weighted distance from v
    vector<long long> ans(N);
    ans[0]=dp[0];
    for(int v:order){
        for(int c:adj[v]){
            if(c==par[v]) continue;
            // moving root from v to c:
            // nodes in subtree(c): distance decreases by 1 each -> -sz[c]
            // nodes outside subtree(c): distance increases by 1 each -> +(totalC - sz[c])
            ans[c]=ans[v]-sz[c]+(totalC-sz[c]);
        }
    }
    cout<<*min_element(ans.begin(),ans.end())<<endl;
}
