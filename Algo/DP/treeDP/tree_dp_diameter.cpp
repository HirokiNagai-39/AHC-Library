// Tree Diameter via Tree DP
// Find the diameter (longest path) of a tree. O(N)
// dp[v] = longest path starting from v going downward.
// Diameter = max over all v of (top two dp values among children).
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
    long long ans=0;
    vector<long long> dp(N,0);
    vector<int> par(N,-1);
    // iterative post-order DFS
    vector<int> order;
    vector<bool> visited(N,false);
    {
        stack<int> st;
        st.push(0); visited[0]=true;
        while(!st.empty()){
            int v=st.top(); st.pop();
            order.push_back(v);
            for(auto [u,w]:adj[v]) if(!visited[u]){
                visited[u]=true; par[u]=v;
                st.push(u);
            }
        }
    }
    reverse(order.begin(),order.end());
    for(int v:order){
        long long best1=0,best2=0; // top two
        for(auto [u,w]:adj[v]){
            if(u==par[v]) continue;
            long long val=dp[u]+w;
            if(val>=best1){ best2=best1; best1=val; }
            else if(val>best2) best2=val;
        }
        dp[v]=best1;
        ans=max(ans,best1+best2);
    }
    cout<<ans<<endl;
}
