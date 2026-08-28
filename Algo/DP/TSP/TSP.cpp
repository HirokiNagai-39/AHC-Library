// 巡回セールスマン問題 (TSP) - bitDP
// N個の都市を全て訪問して戻る最小コスト
// 例題: ABC180 E - Traveling Salesman among Aerial Cities
// https://atcoder.jp/contests/abc180/tasks/abc180_e
// 計算量: O(N^2 * 2^N)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<long long> X(N), Y(N), Z(N);
    for(int i = 0; i < N; i++) cin >> X[i] >> Y[i] >> Z[i];

    // コスト計算 (非対称)
    // dist(i→j) = |Xi-Xj| + |Yi-Yj| + max(0, Zj-Zi)
    vector<vector<long long>> cost(N, vector<long long>(N));
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            cost[i][j] = abs(X[i]-X[j]) + abs(Y[i]-Y[j]) + max(0LL, Z[j]-Z[i]);
        }
    }

    // dp[S][v] = 訪問済み集合S, 現在地v のときの最小コスト
    static constexpr long long INF = 1e18;
    vector<vector<long long>> dp(1 << N, vector<long long>(N, INF));
    dp[1][0] = 0; // 都市0から出発

    for(int S = 1; S < (1 << N); S++){
        for(int v = 0; v < N; v++){
            if(dp[S][v] == INF) continue;
            if(!(S >> v & 1)) continue; // vが訪問済みでなければスキップ
            for(int u = 0; u < N; u++){
                if(S >> u & 1) continue; // uが未訪問
                int nS = S | (1 << u);
                dp[nS][u] = min(dp[nS][u], dp[S][v] + cost[v][u]);
            }
        }
    }

    // 全都市訪問後, 都市0に戻る
    long long ans = INF;
    int full = (1 << N) - 1;
    for(int v = 0; v < N; v++){
        ans = min(ans, dp[full][v] + cost[v][0]);
    }
    cout << ans << endl;
    return 0;
}
