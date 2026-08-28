// ゲームDP (ミニマックス)
// 例題: EDPC L - Deque
// https://atcoder.jp/contests/dp/tasks/dp_l
// 両端から交互に取る. 先手は得点最大化, 後手は先手の得点最小化
// 計算量: O(N^2)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<long long> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];

    // dp[l][r] = a[l..r] が残っているとき, 「手番の人の得点 - 相手の得点」の最大値
    // 手番の人は自分の利益を最大化する
    vector<vector<long long>> dp(N, vector<long long>(N, 0));

    // 長さ1
    for(int i = 0; i < N; i++) dp[i][i] = a[i];

    // 長さ2以上
    for(int len = 2; len <= N; len++){
        for(int l = 0; l + len - 1 < N; l++){
            int r = l + len - 1;
            // 左端を取る: a[l] - dp[l+1][r] (相手のターンになる)
            // 右端を取る: a[r] - dp[l][r-1]
            dp[l][r] = max(a[l] - dp[l+1][r], a[r] - dp[l][r-1]);
        }
    }

    // dp[0][N-1] = 先手の得点 - 後手の得点
    // 先手の得点 = (全体の和 + dp[0][N-1]) / 2
    long long total = 0;
    for(int i = 0; i < N; i++) total += a[i];
    long long X = (total + dp[0][N-1]) / 2; // 先手
    long long Y = total - X;                 // 後手
    cout << X - Y << endl;
    return 0;
}
