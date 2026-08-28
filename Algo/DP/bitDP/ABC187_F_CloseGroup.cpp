// ABC187 F - Close Group
// https://atcoder.jp/contests/abc187/tasks/abc187_f
// Partition N-vertex graph into minimum number of cliques.
// Precompute which subsets are cliques, then subset DP. O(3^N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,M; cin>>N>>M;
    vector<vector<bool>> adj(N,vector<bool>(N,false));
    for(int i=0;i<M;i++){
        int u,v; cin>>u>>v; u--;v--;
        adj[u][v]=adj[v][u]=true;
    }
    int full=1<<N;
    vector<bool> is_clique(full,false);
    for(int S=0;S<full;S++){
        bool ok=true;
        vector<int> vs;
        for(int i=0;i<N;i++) if(S>>i&1) vs.push_back(i);
        for(int i=0;i<(int)vs.size()&&ok;i++)
            for(int j=i+1;j<(int)vs.size()&&ok;j++)
                if(!adj[vs[i]][vs[j]]) ok=false;
        is_clique[S]=ok;
    }
    const int INF=N+1;
    vector<int> dp(full,INF);
    dp[0]=0;
    for(int S=1;S<full;S++){
        // enumerate subsets of S
        for(int T=S;T>0;T=(T-1)&S){
            if(is_clique[T])
                dp[S]=min(dp[S],dp[S^T]+1);
        }
    }
    cout<<dp[full-1]<<endl;
}
