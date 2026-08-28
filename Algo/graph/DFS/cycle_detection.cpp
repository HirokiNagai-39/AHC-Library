// 閉路検出 (有向グラフ / 無向グラフ)
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// ============================================================
// 有向グラフの閉路検出
// DFSの3色分け: 0=未訪問, 1=訪問中(スタック上), 2=完了
// 訪問中の頂点に再度到達 → 閉路
// ============================================================
bool has_cycle_directed(const vector<vector<int>>& g){
    int n = g.size();
    vector<int> state(n, 0);

    function<bool(int)> dfs = [&](int v) -> bool {
        state[v] = 1; // 訪問中
        for(int u : g[v]){
            if(state[u] == 1) return true;  // 閉路発見
            if(state[u] == 0 && dfs(u)) return true;
        }
        state[v] = 2; // 完了
        return false;
    };

    for(int i = 0; i < n; i++){
        if(state[i] == 0 && dfs(i)) return true;
    }
    return false;
}

// ============================================================
// 有向グラフの閉路検出 + 閉路上の頂点を1つ復元
// 閉路が見つかったら cycle_start, cycle_end を記録
// ============================================================
vector<int> find_cycle_directed(const vector<vector<int>>& g){
    int n = g.size();
    vector<int> state(n, 0), par(n, -1);
    int cycle_end = -1, cycle_start = -1;

    function<bool(int)> dfs = [&](int v) -> bool {
        state[v] = 1;
        for(int u : g[v]){
            if(state[u] == 1){
                cycle_end = v;
                cycle_start = u;
                return true;
            }
            if(state[u] == 0){
                par[u] = v;
                if(dfs(u)) return true;
            }
        }
        state[v] = 2;
        return false;
    };

    for(int i = 0; i < n; i++){
        if(state[i] == 0 && dfs(i)) break;
    }

    if(cycle_start == -1) return {}; // 閉路なし

    vector<int> cycle;
    cycle.push_back(cycle_start);
    for(int v = cycle_end; v != cycle_start; v = par[v]){
        cycle.push_back(v);
    }
    cycle.push_back(cycle_start);
    reverse(cycle.begin(), cycle.end());
    return cycle;
}

// ============================================================
// 無向グラフの閉路検出
// 親でない隣接頂点が訪問済み → 閉路
// ============================================================
bool has_cycle_undirected(const vector<vector<int>>& g){
    int n = g.size();
    vector<bool> visited(n, false);

    function<bool(int, int)> dfs = [&](int v, int par) -> bool {
        visited[v] = true;
        for(int u : g[v]){
            if(u == par) continue;      // 来た辺は無視
            if(visited[u]) return true; // 閉路
            if(dfs(u, v)) return true;
        }
        return false;
    };

    for(int i = 0; i < n; i++){
        if(!visited[i] && dfs(i, -1)) return true;
    }
    return false;
}

// === 例題: 有向グラフに閉路があるか判定 ===
int main(){
    int N, M;
    cin >> N >> M;
    vector<vector<int>> g(N);
    for(int i = 0; i < M; i++){
        int a, b; cin >> a >> b;
        a--; b--;
        g[a].push_back(b);
    }

    if(has_cycle_directed(g)){
        cout << "Yes (cycle exists)" << endl;
        auto cycle = find_cycle_directed(g);
        // 閉路をリセットして再度検出 (stateがリセットされないため再構築)
        // find_cycle_directed は独立して使える
    } else {
        cout << "No (DAG)" << endl;
    }
    return 0;
}
