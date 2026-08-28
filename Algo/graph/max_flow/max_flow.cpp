// 最大フロー (Dinic法)
// 例題: 典型的な最大流
// 計算量: O(V^2 * E)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Dinic ===
struct MaxFlow {
    struct Edge {
        int to, rev;
        long long cap;
    };

    int n;
    vector<vector<Edge>> graph;
    vector<int> level, iter;

    MaxFlow(int n) : n(n), graph(n), level(n), iter(n) {}

    void add_edge(int from, int to, long long cap){
        graph[from].push_back({to, (int)graph[to].size(), cap});
        graph[to].push_back({from, (int)graph[from].size() - 1, 0});
    }

    bool bfs(int s, int t){
        fill(level.begin(), level.end(), -1);
        queue<int> q;
        level[s] = 0;
        q.push(s);
        while(!q.empty()){
            int v = q.front(); q.pop();
            for(auto& e : graph[v]){
                if(e.cap > 0 && level[e.to] < 0){
                    level[e.to] = level[v] + 1;
                    q.push(e.to);
                }
            }
        }
        return level[t] >= 0;
    }

    long long dfs(int v, int t, long long f){
        if(v == t) return f;
        for(int& i = iter[v]; i < (int)graph[v].size(); i++){
            Edge& e = graph[v][i];
            if(e.cap > 0 && level[v] < level[e.to]){
                long long d = dfs(e.to, t, min(f, e.cap));
                if(d > 0){
                    e.cap -= d;
                    graph[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    }

    long long max_flow(int s, int t){
        long long flow = 0;
        while(bfs(s, t)){
            fill(iter.begin(), iter.end(), 0);
            long long d;
            while((d = dfs(s, t, LLONG_MAX)) > 0){
                flow += d;
            }
        }
        return flow;
    }
};

// === 例題: 二部マッチング (最大フローで解く) ===
// N人の人とM個の仕事, a[i][j]=1なら人iが仕事jをできる. 最大マッチング数
int main(){
    int N, M;
    cin >> N >> M;
    // 0: source, 1..N: 人, N+1..N+M: 仕事, N+M+1: sink
    int S = 0, T = N + M + 1;
    MaxFlow mf(N + M + 2);

    for(int i = 0; i < N; i++) mf.add_edge(S, i + 1, 1);
    for(int j = 0; j < M; j++) mf.add_edge(N + 1 + j, T, 1);

    for(int i = 0; i < N; i++){
        for(int j = 0; j < M; j++){
            int a;
            cin >> a;
            if(a) mf.add_edge(i + 1, N + 1 + j, 1);
        }
    }

    cout << mf.max_flow(S, T) << endl;
    return 0;
}
