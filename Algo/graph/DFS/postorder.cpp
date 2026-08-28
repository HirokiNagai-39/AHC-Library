// DFS 行きがけ順 / 帰りがけ順
// トポロジカルソートの基盤, オイラーツアーの基盤
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: DFS帰りがけ順 (有向グラフ) ===
// 帰りがけ順の逆順 = トポロジカル順 (DAGの場合)
vector<int> dfs_postorder(const vector<vector<int>>& g){
    int n = g.size();
    vector<bool> visited(n, false);
    vector<int> order;

    function<void(int)> dfs = [&](int v){
        visited[v] = true;
        for(int u : g[v]){
            if(!visited[u]) dfs(u);
        }
        order.push_back(v); // 帰りがけ
    };

    for(int i = 0; i < n; i++){
        if(!visited[i]) dfs(i);
    }
    return order;
}

// === ライブラリ: DFS 行きがけ・帰りがけ時刻 (木) ===
// in_[v]: vに入った時刻, out_[v]: vから出た時刻
// 部分木 = [in_[v], out_[v]) の区間に対応
struct DFSOrder {
    int n, timer;
    vector<vector<int>> g;
    vector<int> in_, out_, depth_;

    DFSOrder(int n) : n(n), timer(0), g(n), in_(n), out_(n), depth_(n, 0) {}

    void add_edge(int u, int v){
        g[u].push_back(v);
        g[v].push_back(u);
    }

    void build(int root = 0){
        timer = 0;
        vector<int> par(n, -1);
        stack<pair<int, bool>> st;
        st.push({root, true});
        while(!st.empty()){
            auto [v, entering] = st.top(); st.pop();
            if(entering){
                in_[v] = timer++;
                st.push({v, false});
                for(int u : g[v]){
                    if(u == par[v]) continue;
                    par[u] = v;
                    depth_[u] = depth_[v] + 1;
                    st.push({u, true});
                }
            } else {
                out_[v] = timer;
            }
        }
    }

    // uがvの祖先か判定
    bool is_ancestor(int u, int v){
        return in_[u] <= in_[v] && out_[v] <= out_[u];
    }
};

// === 例題: 部分木のサイズ ===
int main(){
    int N;
    cin >> N;
    DFSOrder dfs(N);
    for(int i = 0; i < N - 1; i++){
        int u, v; cin >> u >> v;
        u--; v--;
        dfs.add_edge(u, v);
    }
    dfs.build(0);

    // 各頂点の部分木サイズ = out_[v] - in_[v]
    for(int i = 0; i < N; i++){
        cout << "vertex " << i << ": subtree_size=" << dfs.out_[i] - dfs.in_[i]
             << " depth=" << dfs.depth_[i] << "\n";
    }
    return 0;
}
