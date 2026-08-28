// 最小全域木 (Kruskal法)
// 例題: ABC218 E - Destruction (不要な辺の最大コスト)
// https://atcoder.jp/contests/abc218/tasks/abc218_e
// 計算量: O(E log E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Union-Find (Kruskal用) ===
struct UnionFind {
    vector<int> par;
    UnionFind(int n) : par(n) { iota(par.begin(), par.end(), 0); }
    int find(int x){ return par[x] == x ? x : par[x] = find(par[x]); }
    bool unite(int x, int y){
        x = find(x); y = find(y);
        if(x == y) return false;
        par[y] = x;
        return true;
    }
};

// === ライブラリ: Kruskal ===
struct Edge {
    int u, v;
    long long cost;
    bool operator<(const Edge& o) const { return cost < o.cost; }
};

// MSTのコストを返す. mst_edges にMSTの辺を格納
long long kruskal(int n, vector<Edge>& edges, vector<Edge>& mst_edges){
    sort(edges.begin(), edges.end());
    UnionFind uf(n);
    long long total = 0;
    for(auto& e : edges){
        if(uf.unite(e.u, e.v)){
            total += e.cost;
            mst_edges.push_back(e);
        }
    }
    return total;
}

// === 例題: ABC218 E ===
// 報酬が正の辺は全て取り除き, 負の辺だけでMSTを構成
int main(){
    int N, M;
    cin >> N >> M;
    vector<Edge> edges;
    long long positive_sum = 0;

    for(int i = 0; i < M; i++){
        int a, b;
        long long c;
        cin >> a >> b >> c;
        a--; b--;
        if(c >= 0){
            positive_sum += c;
        } else {
            edges.push_back({a, b, c});
        }
    }

    vector<Edge> mst_edges;
    long long mst_cost = kruskal(N, edges, mst_edges);

    // 負の辺のMSTコスト(負) + 正の辺の総和が答え
    cout << positive_sum + mst_cost << endl;
    return 0;
}
