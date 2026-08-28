// 累積和DP (配るDP + 累積和で高速化)
// 例題: EDPC M - Candies
// https://atcoder.jp/contests/dp/tasks/dp_m
// N人の子供にK個のキャンディを配る. 子供iは0~a[i]個もらえる. 配り方の数
// 計算量: O(NK) (累積和で高速化しないと O(NK * max(a)) でTLE)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

int main(){
    int N, K;
    cin >> N >> K;
    vector<int> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];

    // dp[j] = i人目まで決めて合計j個配った場合の数
    vector<long long> dp(K + 1, 0);
    dp[0] = 1;

    for(int i = 0; i < N; i++){
        // 累積和でdpの区間和を高速に取得
        // ndp[j] = Σ dp[j-a[i]] .. dp[j]  (子供iにk個あげる = dp[j-k]の総和, k=0..a[i])
        vector<long long> S(K + 2, 0); // dpの累積和
        for(int j = 0; j <= K; j++){
            S[j + 1] = (S[j] + dp[j]) % MOD;
        }

        vector<long long> ndp(K + 1, 0);
        for(int j = 0; j <= K; j++){
            // dp[max(0, j-a[i])] + ... + dp[j] = S[j+1] - S[max(0, j-a[i])]
            int lo = max(0, j - a[i]);
            ndp[j] = (S[j + 1] - S[lo] + MOD) % MOD;
        }
        dp = ndp;
    }

    cout << dp[K] << endl;
    return 0;
}
