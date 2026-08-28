// オイラーツアー (Euler Tour)
// 木を区間に変換してセグツリー等と組み合わせる
// 例題: ABC202 E (部分木クエリ)
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Euler Tour ===
struct EulerTour {
    int n;
    vector<vector<int>> g;
    vector<int> in_, out_;  // in_[v] = vに入る時刻, out_[v] = vから出る時刻
    vector<int> depth_;
    int timer;

    EulerTour(int n) : n(n), g(n), in_(n), out_(n), depth_(n, 0), timer(0) {}

    void add_edge(int u, int v){
        g[u].push_back(v);
        g[v].push_back(u);
    }

    void build(int root = 0){
        timer = 0;
        // 非再帰DFS
        vector<int> par(n, -1);
        stack<pair<int, bool>> st; // {頂点, 初回訪問か}
        st.push({root, true});
        while(!st.empty()){
            auto [v, first] = st.top(); st.pop();
            if(first){
                in_[v] = timer++;
                st.push({v, false}); // 帰りがけ用
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

    // 頂点vの部分木 → 区間 [in_[v], out_[v])
    pair<int, int> subtree(int v){ return {in_[v], out_[v]}; }
};

// === 例題: 部分木に含まれる深さdの頂点数 ===
int main(){
    int N;
    cin >> N;
    EulerTour et(N);
    for(int i = 1; i < N; i++){
        int p; cin >> p;
        p--;
        et.add_edge(p, i);
    }
    et.build(0);

    // 深さごとに、in_[v] の値をソートして格納
    int max_depth = *max_element(et.depth_.begin(), et.depth_.end());
    vector<vector<int>> by_depth(max_depth + 1);
    for(int i = 0; i < N; i++){
        by_depth[et.depth_[i]].push_back(et.in_[i]);
    }
    for(auto& v : by_depth) sort(v.begin(), v.end());

    int Q;
    cin >> Q;
    while(Q--){
        int u, d;
        cin >> u >> d;
        u--;
        auto [l, r] = et.subtree(u);
        // by_depth[d] の中で [l, r) に含まれる個数
        auto& bv = by_depth[d];
        int cnt = (int)(lower_bound(bv.begin(), bv.end(), r) -
                        lower_bound(bv.begin(), bv.end(), l));
        cout << cnt << "\n";
    }
    return 0;
}
