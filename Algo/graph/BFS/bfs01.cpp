// 01-BFS
// 辺の重みが0か1のグラフの最短経路 (deque使用)
// Dijkstraの O((V+E)logV) より高速な O(V+E) で解ける
// 例題: ABC176 D - Wizard in Maze
// https://atcoder.jp/contests/abc176/tasks/abc176_d
// 計算量: O(V + E) / O(HW)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 01-BFS (グラフ) ===
// g[v] = {(コスト0or1, 行先), ...}
vector<int> bfs01(const vector<vector<pair<int,int>>>& g, int s){
    int n = g.size();
    vector<int> dist(n, INT_MAX);
    deque<int> dq;
    dist[s] = 0;
    dq.push_back(s);
    while(!dq.empty()){
        int v = dq.front(); dq.pop_front();
        for(auto [w, u] : g[v]){
            if(dist[v] + w < dist[u]){
                dist[u] = dist[v] + w;
                if(w == 0) dq.push_front(u);
                else dq.push_back(u);
            }
        }
    }
    return dist;
}

// === 例題: ABC176 D ===
// H*Wの迷路. 隣接マス移動はコスト0, 5*5範囲ワープはコスト1
int main(){
    int H, W;
    cin >> H >> W;
    int sx, sy, gx, gy;
    cin >> sx >> sy >> gx >> gy;
    sx--; sy--; gx--; gy--;
    vector<string> grid(H);
    for(int i = 0; i < H; i++) cin >> grid[i];

    int dx4[] = {0, 0, 1, -1};
    int dy4[] = {1, -1, 0, 0};

    // 01-BFS on grid
    vector<vector<int>> dist(H, vector<int>(W, INT_MAX));
    deque<pair<int,int>> dq;
    dist[sx][sy] = 0;
    dq.push_back({sx, sy});

    while(!dq.empty()){
        auto [x, y] = dq.front(); dq.pop_front();
        // コスト0: 上下左右の隣接マス
        for(int d = 0; d < 4; d++){
            int nx = x + dx4[d], ny = y + dy4[d];
            if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
            if(grid[nx][ny] == '#') continue;
            if(dist[x][y] < dist[nx][ny]){
                dist[nx][ny] = dist[x][y];
                dq.push_front({nx, ny});
            }
        }
        // コスト1: 5*5範囲ワープ
        for(int di = -2; di <= 2; di++){
            for(int dj = -2; dj <= 2; dj++){
                int nx = x + di, ny = y + dj;
                if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
                if(grid[nx][ny] == '#') continue;
                if(dist[x][y] + 1 < dist[nx][ny]){
                    dist[nx][ny] = dist[x][y] + 1;
                    dq.push_back({nx, ny});
                }
            }
        }
    }

    cout << (dist[gx][gy] == INT_MAX ? -1 : dist[gx][gy]) << endl;
    return 0;
}
