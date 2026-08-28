// 最小共通祖先 (LCA: Lowest Common Ancestor)
// ダブリング法
// 例題: ABC014 D - 閉路
// https://atcoder.jp/contests/abc014/tasks/abc014_4
// 計算量: 前処理 O(N log N), クエリ O(log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: LCA (ダブリング) ===
struct LCA {
    int n, LOG;
    vector<vector<int>> g;
    vector<vector<int>> par; // par[k][v] = vの2^k番目の祖先
    vector<int> depth;

    LCA(int n) : n(n), g(n), depth(n, 0) {
        LOG = 1;
        while((1 << LOG) < n) LOG++;
        par.assign(LOG, vector<int>(n, -1));
    }

    void add_edge(int u, int v){
        g[u].push_back(v);
        g[v].push_back(u);
    }

    void build(int root = 0){
        // BFS
        queue<int> q;
        q.push(root);
        par[0][root] = root;
        while(!q.empty()){
            int v = q.front(); q.pop();
            for(int u : g[v]){
                if(par[0][u] != -1 || u == root) continue;
                par[0][u] = v;
                depth[u] = depth[v] + 1;
                q.push(u);
            }
        }
        // ダブリング
        for(int k = 0; k + 1 < LOG; k++){
            for(int v = 0; v < n; v++){
                par[k + 1][v] = par[k][par[k][v]];
            }
        }
    }

    int lca(int u, int v){
        if(depth[u] < depth[v]) swap(u, v);
        // 深さを揃える
        int diff = depth[u] - depth[v];
        for(int k = 0; k < LOG; k++){
            if((diff >> k) & 1) u = par[k][u];
        }
        if(u == v) return u;
        for(int k = LOG - 1; k >= 0; k--){
            if(par[k][u] != par[k][v]){
                u = par[k][u];
                v = par[k][v];
            }
        }
        return par[0][u];
    }

    int dist(int u, int v){
        return depth[u] + depth[v] - 2 * depth[lca(u, v)];
    }
};

// === 例題: ABC014 D ===
// 木のu-v間パスの頂点数 = dist(u,v) + 1
int main(){
    int N;
    cin >> N;
    LCA tree(N);
    for(int i = 0; i < N - 1; i++){
        int u, v;
        cin >> u >> v;
        u--; v--;
        tree.add_edge(u, v);
    }
    tree.build(0);

    int Q;
    cin >> Q;
    while(Q--){
        int a, b;
        cin >> a >> b;
        a--; b--;
        cout << tree.dist(a, b) + 1 << "\n";
    }
    return 0;
}
