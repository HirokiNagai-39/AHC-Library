// EDPC P - Independent Set
// https://atcoder.jp/contests/dp/tasks/dp_p
// 木DP: 木の独立集合を白黒で塗る. 隣接する頂点が両方黒にならない塗り方の数
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

int main(){
    int N;
    cin >> N;
    vector<vector<int>> g(N);
    for(int i = 0; i < N - 1; i++){
        int u, v;
        cin >> u >> v;
        u--; v--;
        g[u].push_back(v);
        g[v].push_back(u);
    }

    // dp[v][0] = vが白のときの塗り方数
    // dp[v][1] = vが黒のときの塗り方数
    vector<array<long long, 2>> dp(N, {1, 1});

    // 根を0としたDFS (非再帰)
    vector<int> order, par(N, -1);
    vector<bool> visited(N, false);
    queue<int> q;
    q.push(0);
    visited[0] = true;
    while(!q.empty()){
        int v = q.front(); q.pop();
        order.push_back(v);
        for(int u : g[v]){
            if(!visited[u]){
                visited[u] = true;
                par[u] = v;
                q.push(u);
            }
        }
    }

    // 葉から根へ
    for(int i = (int)order.size() - 1; i >= 0; i--){
        int v = order[i];
        for(int u : g[v]){
            if(u == par[v]) continue;
            // vが白なら子は白でも黒でもOK
            dp[v][0] = dp[v][0] % MOD * ((dp[u][0] + dp[u][1]) % MOD) % MOD;
            // vが黒なら子は白のみ
            dp[v][1] = dp[v][1] % MOD * dp[u][0] % MOD;
        }
    }
    cout << (dp[0][0] + dp[0][1]) % MOD << endl;
    return 0;
}
