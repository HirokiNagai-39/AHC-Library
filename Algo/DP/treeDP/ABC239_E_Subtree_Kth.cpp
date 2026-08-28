// ABC239 E - Subtree K-th Max
// https://atcoder.jp/contests/abc239/tasks/abc239_e
// For queries (v, k), find the k-th largest value in subtree of v.
// DFS to compute euler tour, then answer queries offline. O(NlogN + Q)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,Q; cin>>N>>Q;
    vector<int> X(N);
    for(int i=0;i<N;i++) cin>>X[i];
    vector<vector<int>> adj(N);
    for(int i=0;i<N-1;i++){
        int u,v; cin>>u>>v; u--;v--;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    // Euler tour to find subtree ranges
    vector<int> in(N),out(N),order;
    int timer=0;
    // iterative DFS
    vector<int> par(N,-1);
    {
        stack<pair<int,bool>> st;
        st.push({0,false});
        vector<bool> visited(N,false);
        visited[0]=true;
        while(!st.empty()){
            auto [v,done]=st.top(); st.pop();
            if(done){ out[v]=timer; continue; }
            in[v]=timer++;
            order.push_back(v);
            st.push({v,true});
            for(int u:adj[v]) if(!visited[u]){
                visited[u]=true; par[u]=v;
                st.push({u,false});
            }
        }
    }
    // For each subtree, collect top-20 values (k<=20)
    // Merge from leaves upward
    vector<vector<int>> top20(N);
    // process in reverse euler order (post-order)
    for(int i=(int)order.size()-1;i>=0;i--){
        int v=order[i];
        top20[v].push_back(X[v]);
        for(int u:adj[v]){
            if(u==par[v]) continue;
            for(int x:top20[u]) top20[v].push_back(x);
        }
        sort(top20[v].rbegin(),top20[v].rend());
        if((int)top20[v].size()>20) top20[v].resize(20);
    }
    while(Q--){
        int v,k; cin>>v>>k; v--;
        cout<<top20[v][k-1]<<"\n";
    }
}
