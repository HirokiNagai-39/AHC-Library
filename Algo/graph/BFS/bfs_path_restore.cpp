// 経路復元つきBFS
// 最短経路を1つ復元する (prev配列で親を記録)
// 例題: ABC088 D 等 (経路復元が必要な問題全般)
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 経路復元つきBFS (グラフ) ===
// 返り値: s→tの最短経路の頂点列. 到達不能なら空
vector<int> bfs_path(const vector<vector<int>>& g, int s, int t){
    int n = g.size();
    vector<int> dist(n, -1), prev_(n, -1);
    queue<int> q;
    dist[s] = 0;
    q.push(s);
    while(!q.empty()){
        int v = q.front(); q.pop();
        for(int u : g[v]){
            if(dist[u] != -1) continue;
            dist[u] = dist[v] + 1;
            prev_[u] = v;
            q.push(u);
        }
    }
    if(dist[t] == -1) return {}; // 到達不能
    // tから逆にたどって復元
    vector<int> path;
    for(int v = t; v != -1; v = prev_[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

// === ライブラリ: 経路復元つきBFS (グリッド) ===
// 返り値: (sx,sy)→(gx,gy) の最短経路の座標列
vector<pair<int,int>> grid_bfs_path(const vector<string>& grid,
                                     int sx, int sy, int gx, int gy){
    int H = grid.size(), W = grid[0].size();
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    vector<vector<int>> dist(H, vector<int>(W, -1));
    vector<vector<pair<int,int>>> prev_(H, vector<pair<int,int>>(W, {-1, -1}));
    queue<pair<int,int>> q;
    dist[sx][sy] = 0;
    q.push({sx, sy});
    while(!q.empty()){
        auto [x, y] = q.front(); q.pop();
        for(int d = 0; d < 4; d++){
            int nx = x + dx[d], ny = y + dy[d];
            if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
            if(grid[nx][ny] == '#' || dist[nx][ny] != -1) continue;
            dist[nx][ny] = dist[x][y] + 1;
            prev_[nx][ny] = {x, y};
            q.push({nx, ny});
        }
    }
    if(dist[gx][gy] == -1) return {};
    vector<pair<int,int>> path;
    for(int x = gx, y = gy; x != -1; ){
        path.push_back({x, y});
        auto [px, py] = prev_[x][y];
        x = px; y = py;
    }
    reverse(path.begin(), path.end());
    return path;
}

// === 例題 ===
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

    int s, t;
    cin >> s >> t;
    s--; t--;
    auto path = bfs_path(g, s, t);

    if(path.empty()){
        cout << -1 << endl;
    } else {
        cout << path.size() - 1 << endl; // 辺数
        for(int v : path) cout << v + 1 << " ";
        cout << endl;
    }
    return 0;
}
