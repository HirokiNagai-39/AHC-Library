// ABC036 D - Coloring
// https://atcoder.jp/contests/abc036/tasks/abc036_d
// Count ways to color tree black/white so no two adjacent nodes are both black.
// Tree DP: dp[v][0]=white, dp[v][1]=black. O(N)
#include <bits/stdc++.h>
using namespace std;
const long long MOD=1e9+7;
int main(){
    int N; cin>>N;
    vector<vector<int>> adj(N);
    for(int i=0;i<N-1;i++){
        int u,v; cin>>u>>v; u--;v--;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    // dp[v][0] = ways if v is white, dp[v][1] = ways if v is black
    vector<array<long long,2>> dp(N);
    vector<bool> visited(N,false);
    // iterative DFS with post-order
    vector<int> order;
    vector<int> par(N,-1);
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
        dp[v][0]=dp[v][1]=1;
        for(int u:adj[v]){
            if(u==par[v]) continue;
            dp[v][0]=dp[v][0]%MOD*(dp[u][0]+dp[u][1])%MOD;
            dp[v][1]=dp[v][1]%MOD*dp[u][0]%MOD;
        }
    }
    cout<<(dp[0][0]+dp[0][1])%MOD<<endl;
}
