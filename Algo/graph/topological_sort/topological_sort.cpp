// トポロジカルソート (DAG上の順序付け)
// 例題: ABC223 D - Restricted Permutation
// https://atcoder.jp/contests/abc223/tasks/abc223_d
// 計算量: O(V + E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: トポロジカルソート (BFS / Kahn's algorithm) ===
// 辞書順最小のトポソを求める (priority_queue使用)
// 辞書順が不要なら普通のqueueでOK
vector<int> topological_sort(const vector<vector<int>>& g, const vector<int>& indeg_){
    int n = g.size();
    vector<int> indeg = indeg_;
    priority_queue<int, vector<int>, greater<int>> pq; // 辞書順最小
    for(int i = 0; i < n; i++){
        if(indeg[i] == 0) pq.push(i);
    }
    vector<int> order;
    while(!pq.empty()){
        int v = pq.top(); pq.pop();
        order.push_back(v);
        for(int u : g[v]){
            if(--indeg[u] == 0) pq.push(u);
        }
    }
    return order; // size < n ならDAGでない (閉路あり)
}

// === 例題: ABC223 D ===
int main(){
    int N, M;
    cin >> N >> M;
    vector<vector<int>> g(N);
    vector<int> indeg(N, 0);
    for(int i = 0; i < M; i++){
        int a, b;
        cin >> a >> b;
        a--; b--;
        g[a].push_back(b);
        indeg[b]++;
    }

    auto order = topological_sort(g, indeg);

    if((int)order.size() != N){
        cout << -1 << endl;
    } else {
        for(int i = 0; i < N; i++){
            cout << order[i] + 1;
            if(i < N - 1) cout << " ";
        }
        cout << endl;
    }
    return 0;
}
