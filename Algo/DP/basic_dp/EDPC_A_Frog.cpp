// 基本DP: EDPC A - Frog 1
// https://atcoder.jp/contests/dp/tasks/dp_a
// N個の足場, 高さh[i]. 足場iから i+1 or i+2 にジャンプ. コスト = |h[i]-h[j]|
// 足場0→N-1の最小コスト
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<int> h(N);
    for(int i = 0; i < N; i++) cin >> h[i];

    vector<int> dp(N, INT_MAX);
    dp[0] = 0;
    for(int i = 0; i < N; i++){
        if(i + 1 < N) dp[i+1] = min(dp[i+1], dp[i] + abs(h[i] - h[i+1]));
        if(i + 2 < N) dp[i+2] = min(dp[i+2], dp[i] + abs(h[i] - h[i+2]));
    }
    cout << dp[N-1] << endl;
    return 0;
}
