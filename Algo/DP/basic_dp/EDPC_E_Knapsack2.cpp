// ナップサック2 (価値ベースDP): EDPC E - Knapsack 2
// https://atcoder.jp/contests/dp/tasks/dp_e
// Wが巨大 (10^9) だが価値の合計が小さい (V*N ≤ 10^5) 場合
// dp[v] = 価値ちょうどvを得る最小重さ
// 計算量: O(N * V_max)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    long long W;
    cin >> N >> W;
    vector<long long> w(N), v(N);
    for(int i = 0; i < N; i++) cin >> w[i] >> v[i];

    int V = 0;
    for(int i = 0; i < N; i++) V += v[i];

    // dp[j] = 価値ちょうどjを達成する最小重さ
    static constexpr long long INF = 1e18;
    vector<long long> dp(V + 1, INF);
    dp[0] = 0;

    for(int i = 0; i < N; i++){
        for(int j = V; j >= v[i]; j--){
            if(dp[j - v[i]] < INF){
                dp[j] = min(dp[j], dp[j - v[i]] + w[i]);
            }
        }
    }

    int ans = 0;
    for(int j = 0; j <= V; j++){
        if(dp[j] <= W) ans = j;
    }
    cout << ans << endl;
    return 0;
}
