// Tree Maximum Matching via Tree DP
// Find maximum matching (set of edges with no shared endpoints) in a tree. O(N)
// dp[v][0] = max matching in subtree of v, v not matched
// dp[v][1] = max matching in subtree of v, v matched with one child
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
    vector<array<int,2>> dp(N); // dp[v][0], dp[v][1]
    vector<int> par(N,-1);
    vector<int> order;
    vector<bool> visited(N,false);
    {
        stack<int> st;
        st.push(0); visited[0]=true;
        while(!st.empty()){
            int v=st.top(); st.pop();
            order.push_back(v);
            for(int u:adj[v]) if(!visited[u]){
                visited[u]=true; par[u]=v;
                st.push(u);
            }
        }
    }
    reverse(order.begin(),order.end());
    for(int v:order){
        dp[v][0]=dp[v][1]=0;
        int sum_children=0;
        int best_gain=-1; // best gain of matching v with a child
        for(int u:adj[v]){
            if(u==par[v]) continue;
            sum_children+=max(dp[u][0],dp[u][1]);
            // gain: match edge (v,u), so u must be unmatched
            int gain=1+dp[u][0]-max(dp[u][0],dp[u][1]);
            best_gain=max(best_gain,gain);
        }
        dp[v][0]=sum_children;
        if(best_gain>=0)
            dp[v][1]=sum_children+best_gain;
    }
    cout<<max(dp[0][0],dp[0][1])<<endl;
}
