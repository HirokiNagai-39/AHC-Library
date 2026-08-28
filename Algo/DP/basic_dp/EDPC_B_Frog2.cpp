// 基本DP: EDPC B - Frog 2
// https://atcoder.jp/contests/dp/tasks/dp_b
// 足場iから i+1, i+2, ..., i+K にジャンプ可能
// 計算量: O(NK)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N, K;
    cin >> N >> K;
    vector<int> h(N);
    for(int i = 0; i < N; i++) cin >> h[i];

    vector<int> dp(N, INT_MAX);
    dp[0] = 0;
    for(int i = 0; i < N; i++){
        for(int j = 1; j <= K && i + j < N; j++){
            dp[i+j] = min(dp[i+j], dp[i] + abs(h[i] - h[i+j]));
        }
    }
    cout << dp[N-1] << endl;
    return 0;
}
