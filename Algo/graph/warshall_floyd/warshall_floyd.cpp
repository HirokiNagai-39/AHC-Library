// Warshall-Floyd法 (全点対最短経路)
// 例題: ABC208 D - Shortest Path Queries 2
// https://atcoder.jp/contests/abc208/tasks/abc208_d
// 計算量: O(V^3)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Warshall-Floyd ===
static constexpr long long INF = 1e18;

struct WarshallFloyd {
    int n;
    vector<vector<long long>> dist;
    vector<vector<int>> nxt; // 経路復元用

    WarshallFloyd(int n) : n(n), dist(n, vector<long long>(n, INF)), nxt(n, vector<int>(n, -1)) {
        for(int i = 0; i < n; i++) dist[i][i] = 0;
    }

    void add_edge(int u, int v, long long w){
        if(w < dist[u][v]){
            dist[u][v] = w;
            nxt[u][v] = v;
        }
    }

    // 実行. 負閉路があれば true を返す
    bool build(){
        for(int k = 0; k < n; k++)
            for(int i = 0; i < n; i++)
                for(int j = 0; j < n; j++)
                    if(dist[i][k] < INF && dist[k][j] < INF &&
                       dist[i][k] + dist[k][j] < dist[i][j]){
                        dist[i][j] = dist[i][k] + dist[k][j];
                        nxt[i][j] = nxt[i][k];
                    }
        for(int i = 0; i < n; i++) if(dist[i][i] < 0) return true;
        return false;
    }

    // s→t の最短経路を復元 (到達不能なら空)
    vector<int> restore_path(int s, int t){
        if(nxt[s][t] == -1) return {};
        vector<int> path = {s};
        while(s != t){ s = nxt[s][t]; path.push_back(s); }
        return path;
    }
};

// === 例題: ABC208 D ===
// 中継点を1..kまでに制限したときの最短距離の総和を求める
int main(){
    int N, M;
    cin >> N >> M;

    // この問題はkを1つずつ追加しながら累積するので直接DPする
    vector<vector<long long>> dist(N, vector<long long>(N, INF));
    for(int i = 0; i < N; i++) dist[i][i] = 0;

    for(int i = 0; i < M; i++){
        int a, b;
        long long c;
        cin >> a >> b >> c;
        a--; b--;
        dist[a][b] = c;
    }

    long long ans = 0;
    for(int k = 0; k < N; k++){
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                if(dist[i][k] < INF && dist[k][j] < INF){
                    dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
                }
                if(i != j && dist[i][j] < INF){
                    ans += dist[i][j];
                }
            }
        }
    }

    cout << ans << endl;
    return 0;
}
