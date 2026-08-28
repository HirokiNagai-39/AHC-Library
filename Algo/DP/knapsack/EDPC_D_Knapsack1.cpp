// EDPC D - Knapsack 1
// https://atcoder.jp/contests/dp/tasks/dp_d
// 0-1ナップサック: N個の品物, 重さW以下で価値最大化
// 計算量: O(NW)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N, W;
    cin >> N >> W;
    vector<long long> w(N), v(N);
    for(int i = 0; i < N; i++) cin >> w[i] >> v[i];

    // dp[j] = 重さj以下で得られる最大価値
    vector<long long> dp(W + 1, 0);
    for(int i = 0; i < N; i++){
        for(int j = W; j >= w[i]; j--){
            dp[j] = max(dp[j], dp[j - w[i]] + v[i]);
        }
    }
    cout << dp[W] << endl;
    return 0;
}
