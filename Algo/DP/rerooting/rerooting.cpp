// 全方位木DP (Rerooting)
// 各頂点を根としたときのDP値を O(N) で全て求める
// 例題: ABC220 F - Distance Sums 2
// https://atcoder.jp/contests/abc220/tasks/abc220_f
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

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

    // Step 1: 根0で木DPして sz[v], dp[v] を求める
    // sz[v] = vの部分木のサイズ
    // dp[v] = vを根としたときの部分木内の距離の総和
    vector<long long> sz(N, 1), dp(N, 0);
    vector<int> order, par(N, -1);
    {
        vector<bool> visited(N, false);
        queue<int> q;
        q.push(0); visited[0] = true;
        while(!q.empty()){
            int v = q.front(); q.pop();
            order.push_back(v);
            for(int u : g[v]) if(!visited[u]){
                visited[u] = true;
                par[u] = v;
                q.push(u);
            }
        }
    }
    // 葉→根
    for(int i = (int)order.size() - 1; i >= 0; i--){
        int v = order[i];
        for(int u : g[v]){
            if(u == par[v]) continue;
            sz[v] += sz[u];
            dp[v] += dp[u] + sz[u]; // 子の距離 + 子の部分木サイズ(各頂点+1)
        }
    }

    // Step 2: Rerooting (根→葉)
    // ans[v] = 全頂点からvまでの距離の総和
    vector<long long> ans(N);
    ans[0] = dp[0];
    for(int i = 0; i < (int)order.size(); i++){
        int v = order[i];
        for(int u : g[v]){
            if(u == par[v]) continue;
            // 根をv→uに移すと:
            // uの部分木側: 距離-1 (sz[u]個)
            // それ以外: 距離+1 (N-sz[u]個)
            ans[u] = ans[v] - sz[u] + (N - sz[u]);
        }
    }

    for(int i = 0; i < N; i++) cout << ans[i] << "\n";
    return 0;
}
