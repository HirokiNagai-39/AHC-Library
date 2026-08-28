// DAG上の経路数
// DAGのs→tパスの総数を求める
// 例題: ABC021 D (変形), グリッドDP の一般化
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

int main(){
    int N, M, s, t;
    cin >> N >> M >> s >> t;
    s--; t--;

    vector<vector<int>> g(N);
    vector<int> indeg(N, 0);
    for(int i = 0; i < M; i++){
        int u, v; cin >> u >> v;
        u--; v--;
        g[u].push_back(v);
        indeg[v]++;
    }

    // トポロジカルソート
    queue<int> q;
    for(int i = 0; i < N; i++){
        if(indeg[i] == 0) q.push(i);
    }
    vector<int> order;
    while(!q.empty()){
        int v = q.front(); q.pop();
        order.push_back(v);
        for(int u : g[v]){
            if(--indeg[u] == 0) q.push(u);
        }
    }

    // dp[v] = sからvへの経路数
    vector<long long> dp(N, 0);
    dp[s] = 1;
    for(int v : order){
        for(int u : g[v]){
            dp[u] = (dp[u] + dp[v]) % MOD;
        }
    }
    cout << dp[t] << endl;
    return 0;
}
