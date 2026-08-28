// 強連結成分分解 (SCC: Strongly Connected Components)
// Kosaraju's algorithm
// 例題: 典型的SCC + DAG上のDP
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: SCC (Kosaraju) ===
struct SCC {
    int n;
    vector<vector<int>> g, rg; // 正グラフ, 逆グラフ
    vector<int> order, comp;   // DFS帰りがけ順, 各頂点の所属成分
    vector<bool> visited;

    SCC(int n) : n(n), g(n), rg(n), comp(n, -1), visited(n, false) {}

    void add_edge(int u, int v){
        g[u].push_back(v);
        rg[v].push_back(u);
    }

    void dfs(int v){
        visited[v] = true;
        for(int u : g[v]) if(!visited[u]) dfs(u);
        order.push_back(v);
    }

    void rdfs(int v, int k){
        comp[v] = k;
        for(int u : rg[v]) if(comp[u] == -1) rdfs(u, k);
    }

    // 成分数を返す. comp[v] = 頂点vの所属成分番号 (トポロジカル順)
    int build(){
        for(int i = 0; i < n; i++) if(!visited[i]) dfs(i);
        int k = 0;
        for(int i = n - 1; i >= 0; i--){
            if(comp[order[i]] == -1) rdfs(order[i], k++);
        }
        return k;
    }

    // 成分間のDAGを構築
    vector<vector<int>> dag(){
        int k = *max_element(comp.begin(), comp.end()) + 1;
        vector<set<int>> edges(k);
        for(int v = 0; v < n; v++){
            for(int u : g[v]){
                if(comp[v] != comp[u]) edges[comp[v]].insert(comp[u]);
            }
        }
        vector<vector<int>> d(k);
        for(int i = 0; i < k; i++){
            d[i] = vector<int>(edges[i].begin(), edges[i].end());
        }
        return d;
    }
};

// === 例題: 頂点数最大のSCC ===
int main(){
    int N, M;
    cin >> N >> M;
    SCC scc(N);
    for(int i = 0; i < M; i++){
        int u, v;
        cin >> u >> v;
        u--; v--;
        scc.add_edge(u, v);
    }

    int k = scc.build();

    // 各成分のサイズ
    vector<int> sz(k, 0);
    for(int i = 0; i < N; i++) sz[scc.comp[i]]++;

    cout << *max_element(sz.begin(), sz.end()) << endl;
    return 0;
}
