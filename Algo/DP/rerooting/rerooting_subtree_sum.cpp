// Rerooting DP Generic Template (Monoid version)
// Computes f(v) for each vertex v as root, where f depends on subtree aggregation.
// Example: sum of distances from v to all other vertices.
// Customize: identity, merge, add_edge, add_vertex functions. O(N)
#include <bits/stdc++.h>
using namespace std;
using T = pair<long long,long long>; // {dist_sum, subtree_size}
T identity(){ return {0,0}; }
T merge(T a, T b){ return {a.first+b.first, a.second+b.second}; }
T add_edge(T a, int v, int u){ // result of subtree u as child of v
    return {a.first+a.second, a.second}; // each node in subtree gets +1 distance
}
T add_vertex(T a, int v){ return {a.first, a.second+1}; }

int main(){
    int N; cin>>N;
    vector<vector<int>> adj(N);
    for(int i=0;i<N-1;i++){
        int u,v; cin>>u>>v; u--;v--;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<int> par(N,-1);
    vector<int> order;
    vector<bool> visited(N,false);
    {
        queue<int> q; q.push(0); visited[0]=true;
        while(!q.empty()){
            int v=q.front(); q.pop(); order.push_back(v);
            for(int u:adj[v]) if(!visited[u]){visited[u]=true;par[u]=v;q.push(u);}
        }
    }
    vector<T> dp(N);
    // bottom-up
    for(int i=N-1;i>=0;i--){
        int v=order[i];
        T agg=identity();
        for(int u:adj[v]) if(u!=par[v])
            agg=merge(agg,add_edge(dp[u],v,u));
        dp[v]=add_vertex(agg,v);
    }
    // top-down rerooting with prefix/suffix
    vector<T> ans(N);
    ans[0]=dp[0];
    for(int v:order){
        vector<int> ch;
        for(int u:adj[v]) if(u!=par[v]) ch.push_back(u);
        int m=ch.size();
        vector<T> pre(m+1,identity()),suf(m+1,identity());
        for(int i=0;i<m;i++) pre[i+1]=merge(pre[i],add_edge(dp[ch[i]],v,ch[i]));
        for(int i=m-1;i>=0;i--) suf[i]=merge(add_edge(dp[ch[i]],v,ch[i]),suf[i+1]);
        for(int i=0;i<m;i++){
            int c=ch[i];
            // dp_v_without_c
            T without=add_vertex(merge(pre[i],suf[i+1]),v);
            // incorporate parent contribution
            // dp[c] should now include edge from c to v with v's subtree = without
            T from_par=add_edge(without,c,v);
            T agg=identity();
            for(int u:adj[c]) if(u!=par[c]&&u!=v) ; // handled below
            // recompute dp[c] with from_par
            T child_agg=identity();
            for(int u:adj[c]) if(u!=v) child_agg=merge(child_agg,add_edge(dp[u],c,u));
            child_agg=merge(child_agg,from_par);
            dp[c]=add_vertex(child_agg,c);
            par[c]=v; // already set
            ans[c]=dp[c];
        }
    }
    for(int i=0;i<N;i++) cout<<ans[i].first<<"\n";
}
