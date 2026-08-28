// グラフDFS (隣接リスト: 再帰版 + 非再帰版)
// 到達可能判定 / 探索順の記録
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: グラフDFS (再帰版) ===
void graph_dfs_rec(const vector<vector<int>>& g, vector<bool>& visited, int v){
    visited[v] = true;
    for(int u : g[v]){
        if(!visited[u]) graph_dfs_rec(g, visited, u);
    }
}

// === ライブラリ: グラフDFS (非再帰版) ===
void graph_dfs_stack(const vector<vector<int>>& g, vector<bool>& visited, int s){
    stack<int> st;
    st.push(s);
    while(!st.empty()){
        int v = st.top(); st.pop();
        if(visited[v]) continue;
        visited[v] = true;
        for(int u : g[v]){
            if(!visited[u]) st.push(u);
        }
    }
}

// === 例題: 頂点sから到達可能な頂点数 ===
int main(){
    int N, M, s;
    cin >> N >> M >> s;
    s--;
    vector<vector<int>> g(N);
    for(int i = 0; i < M; i++){
        int a, b; cin >> a >> b;
        a--; b--;
        g[a].push_back(b);
        g[b].push_back(a);
    }

    vector<bool> visited(N, false);
    graph_dfs_stack(g, visited, s);

    int cnt = 0;
    for(int i = 0; i < N; i++) if(visited[i]) cnt++;
    cout << cnt << endl;
    return 0;
}
