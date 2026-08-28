// 状態付きBFS
// (位置, 状態) の組でBFS. 鍵/壁破壊/方向 等の追加状態を持つ
// 例題: ABC007 C (壁なし基本), ABC176 D (ワープ), JOI系 等
// 計算量: O(HW * 状態数)
#include <bits/stdc++.h>
using namespace std;

static constexpr int dx[] = {0, 0, 1, -1};
static constexpr int dy[] = {1, -1, 0, 0};

// ============================================================
// パターン1: 壁を最大K回壊せるグリッド最短路
// dist[x][y][k] = 壁をk回壊した状態での最短距離
// ============================================================
int bfs_break_wall(const vector<string>& grid, int sx, int sy, int gx, int gy, int K){
    int H = grid.size(), W = grid[0].size();
    vector<vector<vector<int>>> dist(H, vector<vector<int>>(W, vector<int>(K + 1, -1)));
    queue<tuple<int,int,int>> q;
    dist[sx][sy][0] = 0;
    q.push({sx, sy, 0});
    while(!q.empty()){
        auto [x, y, k] = q.front(); q.pop();
        if(x == gx && y == gy) return dist[x][y][k];
        for(int d = 0; d < 4; d++){
            int nx = x + dx[d], ny = y + dy[d];
            if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
            if(grid[nx][ny] == '.' || grid[nx][ny] == 'S' || grid[nx][ny] == 'G'){
                if(dist[nx][ny][k] == -1){
                    dist[nx][ny][k] = dist[x][y][k] + 1;
                    q.push({nx, ny, k});
                }
            } else if(grid[nx][ny] == '#' && k < K){
                if(dist[nx][ny][k + 1] == -1){
                    dist[nx][ny][k + 1] = dist[x][y][k] + 1;
                    q.push({nx, ny, k + 1});
                }
            }
        }
    }
    return -1;
}

// ============================================================
// パターン2: 鍵を拾って扉を開ける
// 鍵がN種類 → 状態 = 2^N のビットマスク
// dist[x][y][bit] = 鍵の所持状態bitでの最短距離
// ============================================================
int bfs_keys(const vector<string>& grid, int sx, int sy, int gx, int gy, int num_keys){
    int H = grid.size(), W = grid[0].size();
    int states = 1 << num_keys;
    vector<vector<vector<int>>> dist(H, vector<vector<int>>(W, vector<int>(states, -1)));
    queue<tuple<int,int,int>> q;
    dist[sx][sy][0] = 0;
    q.push({sx, sy, 0});
    while(!q.empty()){
        auto [x, y, keys] = q.front(); q.pop();
        if(x == gx && y == gy) return dist[x][y][keys];
        for(int d = 0; d < 4; d++){
            int nx = x + dx[d], ny = y + dy[d];
            if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
            char c = grid[nx][ny];
            if(c == '#') continue;
            int nkeys = keys;
            // 小文字 a-z = 鍵を拾う
            if('a' <= c && c < 'a' + num_keys) nkeys |= (1 << (c - 'a'));
            // 大文字 A-Z = 扉 (対応する鍵が必要)
            if('A' <= c && c < 'A' + num_keys){
                if(!(keys >> (c - 'A') & 1)) continue; // 鍵がない
            }
            if(dist[nx][ny][nkeys] == -1){
                dist[nx][ny][nkeys] = dist[x][y][keys] + 1;
                q.push({nx, ny, nkeys});
            }
        }
    }
    return -1;
}

// === 例題: 壁破壊BFS ===
int main(){
    int H, W, K;
    cin >> H >> W >> K;
    vector<string> grid(H);
    int sx, sy, gx, gy;
    for(int i = 0; i < H; i++){
        cin >> grid[i];
        for(int j = 0; j < W; j++){
            if(grid[i][j] == 'S'){ sx = i; sy = j; }
            if(grid[i][j] == 'G'){ gx = i; gy = j; }
        }
    }
    cout << bfs_break_wall(grid, sx, sy, gx, gy, K) << endl;
    return 0;
}
