// Dijkstra法 (単一始点最短経路)
// 例題: ABC362 D - Shortest Path 3 (頂点コスト付き)
// https://atcoder.jp/contests/abc362/tasks/abc362_d
// 計算量: O((V + E) log V)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Dijkstra ===
using P = pair<long long, int>; // {距離, 頂点}

vector<long long> dijkstra(const vector<vector<P>>& g, int s){
    int n = g.size();
    vector<long long> dist(n, LLONG_MAX);
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[s] = 0;
    pq.push({0, s});
    while(!pq.empty()){
        auto [d, v] = pq.top(); pq.pop();
        if(d > dist[v]) continue;
        for(auto [w, u] : g[v]){
            if(dist[v] + w < dist[u]){
                dist[u] = dist[v] + w;
                pq.push({dist[u], u});
            }
        }
    }
    return dist;
}

// === ライブラリ: Dijkstra (経路復元付き) ===
// dist: 最短距離, prev: 経路復元用 (直前の頂点, 始点は-1)
pair<vector<long long>, vector<int>> dijkstra_restore(const vector<vector<P>>& g, int s){
    int n = g.size();
    vector<long long> dist(n, LLONG_MAX);
    vector<int> prev(n, -1);
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[s] = 0;
    pq.push({0, s});
    while(!pq.empty()){
        auto [d, v] = pq.top(); pq.pop();
        if(d > dist[v]) continue;
        for(auto [w, u] : g[v]){
            if(dist[v] + w < dist[u]){
                dist[u] = dist[v] + w;
                prev[u] = v;
                pq.push({dist[u], u});
            }
        }
    }
    return {dist, prev};
}

// prev配列から s→t のパスを復元 (到達不能なら空)
vector<int> restore_path(const vector<int>& prev, int s, int t){
    if(prev[t] == -1 && s != t) return {}; // 到達不能
    vector<int> path;
    for(int v = t; v != -1; v = prev[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

// === 例題: ABC362 D ===
int main(){
    int N, M;
    cin >> N >> M;
    vector<long long> A(N);
    for(int i = 0; i < N; i++) cin >> A[i];

    vector<vector<P>> g(N);
    for(int i = 0; i < M; i++){
        int u, v;
        long long w;
        cin >> u >> v >> w;
        u--; v--;
        // 辺のコスト + 行先の頂点コスト
        g[u].push_back({w + A[v], v});
        g[v].push_back({w + A[u], u});
    }

    auto dist = dijkstra(g, 0);
    // 頂点0のコストを始点に加算
    for(int i = 1; i < N; i++){
        cout << dist[i] + A[0];
        if(i < N - 1) cout << " ";
    }
    cout << endl;
    return 0;
}
