// Prim法 (最小全域木)
// 隣接リスト版. priority_queue使用
// 例題: GRL_2_A (最小全域木)
// 計算量: O(E log V)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Prim ===
struct Edge {
    int u, v;
    long long cost;
};

// MSTのコストを返す. mst_edges にMSTの辺を格納
// g[v] = {{cost, to}, ...}
long long prim(int n, const vector<vector<pair<long long, int>>>& g, vector<Edge>& mst_edges){
    // {cost, to, from}
    using T = tuple<long long, int, int>;
    vector<bool> used(n, false);
    priority_queue<T, vector<T>, greater<T>> pq;
    pq.push({0, 0, -1});
    long long total = 0;
    while(!pq.empty()){
        auto [w, v, from] = pq.top(); pq.pop();
        if(used[v]) continue;
        used[v] = true;
        total += w;
        if(from != -1){
            mst_edges.push_back({from, v, w});
        }
        for(auto [c, u] : g[v]){
            if(!used[u]) pq.push({c, u, v});
        }
    }
    return total;
}

// === 例題: 最小全域木のコストと辺の出力 ===
int main(){
    int N, M;
    cin >> N >> M;
    vector<vector<pair<long long, int>>> g(N);
    for(int i = 0; i < M; i++){
        int a, b;
        long long c;
        cin >> a >> b >> c;
        a--; b--;
        g[a].push_back({c, b});
        g[b].push_back({c, a});
    }

    vector<Edge> mst_edges;
    long long cost = prim(N, g, mst_edges);

    cout << cost << "\n";
    // MSTを構成する辺の出力
    for(auto& e : mst_edges){
        cout << e.u + 1 << " " << e.v + 1 << " " << e.cost << "\n";
    }
    return 0;
}
