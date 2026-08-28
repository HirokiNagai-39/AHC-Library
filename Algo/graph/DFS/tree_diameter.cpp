// 木の直径 (BFS 2回)
// 木の最遠頂点ペア間の距離
// 例題: ABC019 D (要ログイン), 典型問題
// 計算量: O(V)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 木の直径 ===
// 1. 任意の頂点からBFSして最遠点uを求める
// 2. uからBFSして最遠点vを求める
// u-v間の距離が直径

// BFSで最遠頂点とその距離を返す
pair<int,int> farthest(const vector<vector<int>>& g, int s){
    int n = g.size();
    vector<int> dist(n, -1);
    queue<int> q;
    dist[s] = 0;
    q.push(s);
    int far_v = s, far_d = 0;
    while(!q.empty()){
        int v = q.front(); q.pop();
        for(int u : g[v]){
            if(dist[u] != -1) continue;
            dist[u] = dist[v] + 1;
            q.push(u);
            if(dist[u] > far_d){
                far_d = dist[u];
                far_v = u;
            }
        }
    }
    return {far_v, far_d};
}

int tree_diameter(const vector<vector<int>>& g){
    auto [v1, d1] = farthest(g, 0);    // 任意の頂点から最遠点
    auto [v2, d2] = farthest(g, v1);   // その最遠点からさらに最遠点
    return d2;
}

// === ライブラリ: 重み付き木の直径 ===
// 辺に重みがある場合
using P = pair<long long, int>; // {距離, 頂点}

pair<int, long long> farthest_weighted(const vector<vector<P>>& g, int s){
    int n = g.size();
    vector<long long> dist(n, -1);
    queue<int> q;
    dist[s] = 0;
    q.push(s);
    int far_v = s;
    long long far_d = 0;
    while(!q.empty()){
        int v = q.front(); q.pop();
        for(auto [w, u] : g[v]){
            if(dist[u] != -1) continue;
            dist[u] = dist[v] + w;
            q.push(u);
            if(dist[u] > far_d){
                far_d = dist[u];
                far_v = u;
            }
        }
    }
    return {far_v, far_d};
}

long long tree_diameter_weighted(const vector<vector<P>>& g){
    auto [v1, d1] = farthest_weighted(g, 0);
    auto [v2, d2] = farthest_weighted(g, v1);
    return d2;
}

// === 例題: 木の直径 ===
int main(){
    int N;
    cin >> N;
    vector<vector<int>> g(N);
    for(int i = 0; i < N - 1; i++){
        int u, v; cin >> u >> v;
        u--; v--;
        g[u].push_back(v);
        g[v].push_back(u);
    }
    cout << tree_diameter(g) << endl;
    return 0;
}
