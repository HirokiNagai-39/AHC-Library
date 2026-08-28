// 多始点BFS (Multi-source BFS)
// 複数の始点から同時にBFS. 各セルへの最短距離を求める
// 例題: ABC176 D - Wizard in Maze (多始点BFS + 01-BFS の複合)
// 応用: 最寄りの特定セルまでの距離, 火の広がりシミュレーション
// 計算量: O(HW)
#include <bits/stdc++.h>
using namespace std;

static constexpr int dx[] = {0, 0, 1, -1};
static constexpr int dy[] = {1, -1, 0, 0};

// === ライブラリ: 多始点BFS (グリッド) ===
// source_charのセルを全て始点として距離0でBFS
vector<vector<int>> multi_source_bfs(const vector<string>& grid, char source_char){
    int H = grid.size(), W = grid[0].size();
    vector<vector<int>> dist(H, vector<int>(W, -1));
    queue<pair<int,int>> q;
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            if(grid[i][j] == source_char){
                dist[i][j] = 0;
                q.push({i, j});
            }
        }
    }
    while(!q.empty()){
        auto [x, y] = q.front(); q.pop();
        for(int d = 0; d < 4; d++){
            int nx = x + dx[d], ny = y + dy[d];
            if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
            if(grid[nx][ny] == '#' || dist[nx][ny] != -1) continue;
            dist[nx][ny] = dist[x][y] + 1;
            q.push({nx, ny});
        }
    }
    return dist;
}

// === ライブラリ: 多始点BFS (グラフ, 始点リスト指定) ===
vector<int> multi_source_bfs_graph(const vector<vector<int>>& g, const vector<int>& sources){
    int n = g.size();
    vector<int> dist(n, -1);
    queue<int> q;
    for(int s : sources){
        dist[s] = 0;
        q.push(s);
    }
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

// === 例題: 火の広がりシミュレーション ===
// 人(S)が出口(G)に火(F)より先に到達できるか
// 1. 火の多始点BFSで各セルに火が到達する時刻を求める
// 2. 人のBFSで, 到着時刻 < 火の到着時刻 のセルだけ通る
int main(){
    int H, W;
    cin >> H >> W;
    vector<string> grid(H);
    int sx, sy, gx, gy;
    for(int i = 0; i < H; i++){
        cin >> grid[i];
        for(int j = 0; j < W; j++){
            if(grid[i][j] == 'S'){ sx = i; sy = j; }
            if(grid[i][j] == 'G'){ gx = i; gy = j; }
        }
    }

    // 火の到達時刻
    auto fire = multi_source_bfs(grid, 'F');

    // 人のBFS (火より先に到達できるセルのみ通る)
    vector<vector<int>> dist(H, vector<int>(W, -1));
    queue<pair<int,int>> q;
    dist[sx][sy] = 0;
    q.push({sx, sy});
    while(!q.empty()){
        auto [x, y] = q.front(); q.pop();
        for(int d = 0; d < 4; d++){
            int nx = x + dx[d], ny = y + dy[d];
            if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
            if(grid[nx][ny] == '#' || dist[nx][ny] != -1) continue;
            int t = dist[x][y] + 1;
            // 火が到達していない or 火より先に着ける
            if(fire[nx][ny] != -1 && t >= fire[nx][ny]) continue;
            dist[nx][ny] = t;
            q.push({nx, ny});
        }
    }

    if(dist[gx][gy] == -1) cout << "IMPOSSIBLE" << endl;
    else cout << dist[gx][gy] << endl;
    return 0;
}
