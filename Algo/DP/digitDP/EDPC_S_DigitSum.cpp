// EDPC S - Digit Sum
// https://atcoder.jp/contests/dp/tasks/dp_s
// 桁DP: 1以上K以下の整数で, 各桁の和がD倍数であるものの個数
// 計算量: O(|K| * D * 2)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

int main(){
    string K;
    int D;
    cin >> K >> D;
    int n = K.size();

    // dp[i][j][tight]
    // i桁目まで決定, 桁和 mod D = j, tight: 上限制約あり
    // メモリ節約のため1次元圧縮
    vector<vector<long long>> dp(D, vector<long long>(2, 0));
    dp[0][1] = 1; // 初期状態: 0桁, 余り0, tight

    for(int i = 0; i < n; i++){
        vector<vector<long long>> ndp(D, vector<long long>(2, 0));
        int lim = K[i] - '0';
        for(int j = 0; j < D; j++){
            // tight = 0 の場合: 0-9 すべて選べる
            if(dp[j][0]){
                for(int d = 0; d <= 9; d++){
                    ndp[(j + d) % D][0] = (ndp[(j + d) % D][0] + dp[j][0]) % MOD;
                }
            }
            // tight = 1 の場合
            if(dp[j][1]){
                for(int d = 0; d <= lim; d++){
                    int nt = (d == lim) ? 1 : 0;
                    ndp[(j + d) % D][nt] = (ndp[(j + d) % D][nt] + dp[j][1]) % MOD;
                }
            }
        }
        dp = ndp;
    }
    // 答え = dp[0][0] + dp[0][1] - 1 (0を除く)
    long long ans = (dp[0][0] + dp[0][1] - 1 + MOD) % MOD;
    cout << ans << endl;
    return 0;
}
