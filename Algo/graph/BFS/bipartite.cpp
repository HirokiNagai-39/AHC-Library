// 二部グラフ判定 (BFS)
// グラフを2色で塗り分けられるか判定
// 例題: ABC327 D - Good Tuple Problem
// https://atcoder.jp/contests/abc327/tasks/abc327_d
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 二部グラフ判定 ===
// color[v] = 0 or 1 で2色に塗り分ける
// 二部グラフでなければ false を返す
bool is_bipartite(const vector<vector<int>>& g, vector<int>& color){
    int n = g.size();
    color.assign(n, -1);
    for(int s = 0; s < n; s++){
        if(color[s] != -1) continue;
        color[s] = 0;
        queue<int> q;
        q.push(s);
        while(!q.empty()){
            int v = q.front(); q.pop();
            for(int u : g[v]){
                if(color[u] == -1){
                    color[u] = color[v] ^ 1;
                    q.push(u);
                } else if(color[u] == color[v]){
                    return false; // 奇数長閉路 → 二部グラフでない
                }
            }
        }
    }
    return true;
}

// === 例題: ABC327 D ===
int main(){
    int N, M;
    cin >> N >> M;
    vector<int> A(M), B(M);
    for(int i = 0; i < M; i++) cin >> A[i], A[i]--;
    for(int i = 0; i < M; i++) cin >> B[i], B[i]--;

    vector<vector<int>> g(N);
    for(int i = 0; i < M; i++){
        g[A[i]].push_back(B[i]);
        g[B[i]].push_back(A[i]);
    }

    vector<int> color;
    cout << (is_bipartite(g, color) ? "Yes" : "No") << endl;
    return 0;
}
