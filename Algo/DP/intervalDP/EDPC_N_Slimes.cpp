// EDPC N - Slimes
// https://atcoder.jp/contests/dp/tasks/dp_n
// 区間DP: N個のスライムを合体. 合体コスト = 2つの重さの和. 最小コスト
// 計算量: O(N^3)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<long long> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];

    // 累積和
    vector<long long> S(N + 1, 0);
    for(int i = 0; i < N; i++) S[i + 1] = S[i] + a[i];

    // dp[l][r] = スライム l..r を1つにまとめる最小コスト
    vector<vector<long long>> dp(N, vector<long long>(N, 0));

    for(int len = 2; len <= N; len++){
        for(int l = 0; l + len - 1 < N; l++){
            int r = l + len - 1;
            dp[l][r] = LLONG_MAX;
            long long sum_lr = S[r + 1] - S[l];
            for(int k = l; k < r; k++){
                dp[l][r] = min(dp[l][r], dp[l][k] + dp[k + 1][r] + sum_lr);
            }
        }
    }
    cout << dp[0][N - 1] << endl;
    return 0;
}
