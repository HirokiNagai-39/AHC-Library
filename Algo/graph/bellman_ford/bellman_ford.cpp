// Bellman-Ford法 (負辺を含む最短経路 / 負閉路検出)
// 例題: ABC061 D - Score Attack
// https://atcoder.jp/contests/abc061/tasks/abc061_d
// 計算量: O(VE)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Bellman-Ford ===
struct Edge { int from, to; long long cost; };

// 負閉路がある場合 true を返す
// dist[i] = 始点sからiへの最短距離 (到達不能は LLONG_MAX)
bool bellman_ford(int n, int s, const vector<Edge>& edges, vector<long long>& dist){
    dist.assign(n, LLONG_MAX);
    dist[s] = 0;
    for(int i = 0; i < n; i++){
        bool updated = false;
        for(auto& e : edges){
            if(dist[e.from] == LLONG_MAX) continue;
            if(dist[e.from] + e.cost < dist[e.to]){
                dist[e.to] = dist[e.from] + e.cost;
                updated = true;
                if(i == n - 1) return true; // 負閉路
            }
        }
        if(!updated) break;
    }
    return false;
}

// 頂点tに影響する負閉路があるか判定
bool has_negative_cycle_to(int n, int t, const vector<Edge>& edges, vector<long long>& dist){
    // n-1回の後、さらにn-1回更新して、tが更新されるか
    for(int i = 0; i < n - 1; i++){
        for(auto& e : edges){
            if(dist[e.from] == LLONG_MAX) continue;
            if(dist[e.from] + e.cost < dist[e.to]){
                dist[e.to] = dist[e.from] + e.cost;
            }
        }
    }
    // 最初のn-1回後のdist[t]と比較して変化があれば負閉路の影響あり
    return false; // 上の関数でまとめて判定推奨
}

// === 例題: ABC061 D ===
// コストを-1倍して最長経路 → Bellman-Fordで負閉路検出
int main(){
    int N, M;
    cin >> N >> M;
    vector<Edge> edges(M);
    for(int i = 0; i < M; i++){
        cin >> edges[i].from >> edges[i].to >> edges[i].cost;
        edges[i].from--; edges[i].to--;
        edges[i].cost = -edges[i].cost; // 最長 → 最短に反転
    }

    vector<long long> dist;
    // まずn-1回緩和
    dist.assign(N, LLONG_MAX);
    dist[0] = 0;
    for(int i = 0; i < N - 1; i++){
        for(auto& e : edges){
            if(dist[e.from] == LLONG_MAX) continue;
            if(dist[e.from] + e.cost < dist[e.to]){
                dist[e.to] = dist[e.from] + e.cost;
            }
        }
    }
    long long ans = dist[N - 1];

    // さらにn-1回緩和してN-1に到達する負閉路があるか
    for(int i = 0; i < N - 1; i++){
        for(auto& e : edges){
            if(dist[e.from] == LLONG_MAX) continue;
            if(dist[e.from] + e.cost < dist[e.to]){
                dist[e.to] = dist[e.from] + e.cost;
            }
        }
    }

    if(dist[N - 1] < ans){
        cout << "inf" << endl;
    } else {
        cout << -ans << endl;
    }
    return 0;
}
