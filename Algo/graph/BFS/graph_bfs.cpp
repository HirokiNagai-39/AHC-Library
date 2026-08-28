// グラフBFS (隣接リスト, 最短距離)
// 例題: ABC168 D - .. (Double Dots)
// https://atcoder.jp/contests/abc168/tasks/abc168_d
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: グラフBFS ===
// dist[v] = 始点sからの最短距離, -1なら到達不能
vector<int> graph_bfs(const vector<vector<int>>& g, int s){
    int n = g.size();
    vector<int> dist(n, -1);
    queue<int> q;
    dist[s] = 0;
    q.push(s);
    while(!q.empty()){
        int v = q.front(); q.pop();
        for(int u : g[v]){
            if(dist[u] != -1) continue;
            dist[u] = dist[v] + 1;
            q.push(u);
        }
    }
    return dist;
}

// === 例題: ABC168 D ===
// 頂点1からの最短路木を構成し, 各頂点の親(道しるべ)を出力
int main(){
    int N, M;
    cin >> N >> M;
    vector<vector<int>> g(N);
    for(int i = 0; i < M; i++){
        int a, b; cin >> a >> b;
        a--; b--;
        g[a].push_back(b);
        g[b].push_back(a);
    }

    // BFS で親を記録
    vector<int> dist(N, -1), par(N, -1);
    queue<int> q;
    dist[0] = 0;
    q.push(0);
    while(!q.empty()){
        int v = q.front(); q.pop();
        for(int u : g[v]){
            if(dist[u] != -1) continue;
            dist[u] = dist[v] + 1;
            par[u] = v;
            q.push(u);
        }
    }

    cout << "Yes" << endl;
    for(int i = 1; i < N; i++) cout << par[i] + 1 << "\n";
    return 0;
}
