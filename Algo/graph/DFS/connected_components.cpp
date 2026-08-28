// 連結成分の個数 / ラベリング
// 例題: ABC284 C - Count Connected Components
// https://atcoder.jp/contests/abc284/tasks/abc284_c
// 計算量: O(V + E) / O(HW)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 連結成分 (グラフ版) ===
// 連結成分数を返し, comp[v] に所属成分番号を格納
int count_components(const vector<vector<int>>& g, vector<int>& comp){
    int n = g.size();
    comp.assign(n, -1);
    int cnt = 0;
    for(int s = 0; s < n; s++){
        if(comp[s] != -1) continue;
        queue<int> q;
        q.push(s);
        comp[s] = cnt;
        while(!q.empty()){
            int v = q.front(); q.pop();
            for(int u : g[v]){
                if(comp[u] == -1){
                    comp[u] = cnt;
                    q.push(u);
                }
            }
        }
        cnt++;
    }
    return cnt;
}

// === ライブラリ: 連結成分 (グリッド版) ===
// '.' の連結成分数を数える
int count_grid_components(const vector<string>& grid){
    int H = grid.size(), W = grid[0].size();
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    vector<vector<bool>> visited(H, vector<bool>(W, false));
    int cnt = 0;
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            if(grid[i][j] == '#' || visited[i][j]) continue;
            // BFS で塗りつぶし
            queue<pair<int,int>> q;
            q.push({i, j});
            visited[i][j] = true;
            while(!q.empty()){
                auto [x, y] = q.front(); q.pop();
                for(int d = 0; d < 4; d++){
                    int nx = x + dx[d], ny = y + dy[d];
                    if(nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
                    if(grid[nx][ny] == '#' || visited[nx][ny]) continue;
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
            cnt++;
        }
    }
    return cnt;
}

// === 例題: ABC284 C ===
int main(){
    int N, M;
    cin >> N >> M;
    vector<vector<int>> g(N);
    for(int i = 0; i < M; i++){
        int u, v; cin >> u >> v;
        u--; v--;
        g[u].push_back(v);
        g[v].push_back(u);
    }

    vector<int> comp;
    cout << count_components(g, comp) << endl;
    return 0;
}
