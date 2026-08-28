// グリッドDFS (再帰版 + 非再帰版)
// 到達可能判定 / 塗りつぶし
// 例題: ATC001 A - 深さ優先探索
// https://atcoder.jp/contests/atc001/tasks/dfs_a
// 計算量: O(HW)
#include <bits/stdc++.h>
using namespace std;

static constexpr int dx[] = {0, 0, 1, -1};
static constexpr int dy[] = {1, -1, 0, 0};

// === ライブラリ: グリッドDFS (再帰版) ===
// 注意: グリッドが大きい(~10^6)と再帰でスタックオーバーフローの恐れ
//       → その場合は非再帰版かBFSを使う
void grid_dfs_rec(const vector<string>& grid, vector<vector<bool>>& visited,
                  int x, int y){
    int H = grid.size(), W = grid[0].size();
    visited[x][y] = true;
    for(int d = 0; d < 4; d++){
        int nx = x + dx[d], ny = y + dy[d];
        if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
        if(grid[nx][ny] == '#' || visited[nx][ny]) continue;
        grid_dfs_rec(grid, visited, nx, ny);
    }
}

// === ライブラリ: グリッドDFS (非再帰・スタック版) ===
// スタックオーバーフローを回避. 大きいグリッドでも安全
void grid_dfs_stack(const vector<string>& grid, vector<vector<bool>>& visited,
                    int sx, int sy){
    int H = grid.size(), W = grid[0].size();
    stack<pair<int,int>> st;
    st.push({sx, sy});
    visited[sx][sy] = true;
    while(!st.empty()){
        auto [x, y] = st.top(); st.pop();
        for(int d = 0; d < 4; d++){
            int nx = x + dx[d], ny = y + dy[d];
            if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
            if(grid[nx][ny] == '#' || visited[nx][ny]) continue;
            visited[nx][ny] = true; // push時にマーク (重複push防止)
            st.push({nx, ny});
        }
    }
}

// === 例題: ATC001 A ===
int main(){
    int H, W;
    cin >> H >> W;
    vector<string> grid(H);
    int sx, sy, gx, gy;
    for(int i = 0; i < H; i++){
        cin >> grid[i];
        for(int j = 0; j < W; j++){
            if(grid[i][j] == 's'){ sx = i; sy = j; }
            if(grid[i][j] == 'g'){ gx = i; gy = j; }
        }
    }

    vector<vector<bool>> visited(H, vector<bool>(W, false));
    grid_dfs_stack(grid, visited, sx, sy);
    cout << (visited[gx][gy] ? "Yes" : "No") << endl;
    return 0;
}
