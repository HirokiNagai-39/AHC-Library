// EDPC O - Matching
// https://atcoder.jp/contests/dp/tasks/dp_o
// bitDP: N人の男とN人の女のマッチング数を数える
// 計算量: O(N * 2^N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

int main(){
    int N;
    cin >> N;
    vector<vector<int>> a(N, vector<int>(N));
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++)
            cin >> a[i][j];

    // dp[S] = 男0..(__builtin_popcount(S)-1)までが,
    //         女の集合Sとマッチングする場合の数
    vector<long long> dp(1 << N, 0);
    dp[0] = 1;
    for(int S = 0; S < (1 << N); S++){
        int i = __builtin_popcount(S); // 次にマッチさせる男
        if(i >= N) continue;
        for(int j = 0; j < N; j++){
            if(S >> j & 1) continue;
            if(!a[i][j]) continue;
            dp[S | (1 << j)] = (dp[S | (1 << j)] + dp[S]) % MOD;
        }
    }
    cout << dp[(1 << N) - 1] << endl;
    return 0;
}
