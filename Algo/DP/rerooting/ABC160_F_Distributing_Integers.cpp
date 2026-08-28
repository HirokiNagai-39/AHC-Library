// ABC160 F - Distributing Integers
// https://atcoder.jp/contests/abc160/tasks/abc160_f
// For each vertex as root, count labeled rooted tree orderings.
// Rerooting DP. ans[v] = N! / prod(sz[u]!) for subtree sizes from v.
#include <bits/stdc++.h>
using namespace std;
const int MOD=1e9+7;
long long pw(long long a,long long b,long long m){long long r=1;a%=m;while(b>0){if(b&1)r=r*a%m;a=a*a%m;b>>=1;}return r;}
long long inv(long long a){return pw(a,MOD-2,MOD);}
int main(){
    int N; cin>>N;
    vector<vector<int>> adj(N);
    for(int i=0;i<N-1;i++){
        int u,v; cin>>u>>v; u--;v--;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<long long> fac(N+1),ifac(N+1);
    fac[0]=1; for(int i=1;i<=N;i++) fac[i]=fac[i-1]*i%MOD;
    ifac[N]=inv(fac[N]); for(int i=N-1;i>=0;i--) ifac[i]=ifac[i+1]*(i+1)%MOD;
    // Root at 0, compute subtree sizes
    vector<int> par(N,-1),sz(N,1);
    vector<int> order;
    vector<bool> visited(N,false);
    {
        queue<int> q; q.push(0); visited[0]=true;
        while(!q.empty()){
            int v=q.front(); q.pop(); order.push_back(v);
            for(int u:adj[v]) if(!visited[u]){visited[u]=true;par[u]=v;q.push(u);}
        }
    }
    for(int i=N-1;i>=0;i--){ int v=order[i]; if(par[v]>=0) sz[par[v]]+=sz[v]; }
    // ans[0] = (N-1)! / prod(sz[child]!) ... actually ans[v] = fac[N-1] * prod(ifac[sz[u]]) for children u
    // For root 0: ans = fac[N-1] * prod_{u!=0} ifac[sz[u]]
    // Rerooting: when root changes from v to child c, sz[c] becomes N-sz[c], sz[v] becomes sz[v]-sz_old[c]=N-sz[c]...
    // Actually: ans[v] = fac[N-1] * prod_{all nodes u!=v} ifac[sz_v[u]]
    // where sz_v[u] is subtree size of u when v is root.
    // Simpler: dp[v] = prod_{children c of v} ifac[sz[c]]
    // ans[v] = fac[N-1] * dp[v]
    // Rerooting: moving root from v to child c:
    //   dp[c] *= ifac[N - sz[c]]  (v becomes child of c with size N-sz[c])
    //   dp[v] /= ifac[sz[c]]
    vector<long long> dp(N,1);
    for(int i=N-1;i>=0;i--){
        int v=order[i];
        for(int u:adj[v]) if(u!=par[v])
            dp[v]=dp[v]%MOD*ifac[sz[u]]%MOD*dp[u]%MOD;
    }
    // now dp[0] has product for root=0
    // rerooting BFS
    vector<long long> ans(N);
    ans[0]=fac[N-1]%MOD*dp[0]%MOD;
    for(int v:order){
        for(int c:adj[v]){
            if(c==par[v]) continue;
            // remove c's contribution from v, add v's contribution to c
            // dp_v_without_c = dp[v] * inv(ifac[sz[c]] * dp[c])
            // dp[c]_new = dp[c] * ifac[N-sz[c]] * dp_v_without_c
            long long dpv_without = dp[v] % MOD * inv(ifac[sz[c]]%MOD*dp[c]%MOD) % MOD;
            dp[c] = dp[c] % MOD * ifac[N-sz[c]] % MOD * dpv_without % MOD;
            ans[c] = fac[N-1] % MOD * dp[c] % MOD;
        }
    }
    for(int i=0;i<N;i++) cout<<ans[i]<<"\n";
}
