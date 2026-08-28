// DAG上のDP (最長路)
// 例題: EDPC G - Longest Path
// https://atcoder.jp/contests/dp/tasks/dp_g
// 有向グラフ(DAG)上の最長パスの辺数
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: DAG上のDP ===
// トポロジカル順にDPする
// dp[v] = 頂点vを終点とする最長パスの辺数

int main(){
    int N, M;
    cin >> N >> M;
    vector<vector<int>> g(N);
    vector<int> indeg(N, 0);
    for(int i = 0; i < M; i++){
        int u, v; cin >> u >> v;
        u--; v--;
        g[u].push_back(v);
        indeg[v]++;
    }

    // トポロジカルソート (Kahn)
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

    // トポロジカル順にDP
    vector<int> dp(N, 0);
    int ans = 0;
    for(int v : order){
        for(int u : g[v]){
            dp[u] = max(dp[u], dp[v] + 1);
            ans = max(ans, dp[u]);
        }
    }
    cout << ans << endl;
    return 0;
}
