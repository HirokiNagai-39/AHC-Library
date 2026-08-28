// グリッドBFS (最短距離)
// 例題: ABC088 D - Grid
// https://atcoder.jp/contests/abc088/tasks/abc088_d
// 計算量: O(HW)
#include <bits/stdc++.h>
using namespace std;

static constexpr int dx[] = {0, 0, 1, -1};
static constexpr int dy[] = {1, -1, 0, 0};
// 8方向の場合: {0,1},{0,-1},{1,0},{-1,0},{1,1},{1,-1},{-1,1},{-1,-1}

// === ライブラリ: グリッドBFS ===
// dist[i][j] = (sx,sy)からの最短距離, -1なら到達不能
vector<vector<int>> grid_bfs(const vector<string>& grid, int sx, int sy){
    int H = grid.size(), W = grid[0].size();
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
            dist[nx][ny] = dist[x][y] + 1;
            q.push({nx, ny});
        }
    }
    return dist;
}

// === 例題: ABC088 D ===
int main(){
    int H, W;
    cin >> H >> W;
    vector<string> grid(H);
    for(int i = 0; i < H; i++) cin >> grid[i];

    int white = 0;
    for(int i = 0; i < H; i++)
        for(int j = 0; j < W; j++)
            if(grid[i][j] == '.') white++;

    auto dist = grid_bfs(grid, 0, 0);

    if(dist[H-1][W-1] == -1) cout << -1 << endl;
    else cout << white - dist[H-1][W-1] - 1 << endl;
    return 0;
}
