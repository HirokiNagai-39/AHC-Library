// EDPC I - Coins
// https://atcoder.jp/contests/dp/tasks/dp_i
// N coins with heads probability p[i]. Find probability that heads > tails.
// dp[j] = probability of exactly j heads after processing coins. O(N^2)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<double> p(N);
    for(auto &x : p) cin >> x;

    vector<double> dp(N + 1, 0);
    dp[0] = 1.0;
    for(int i = 0; i < N; i++){
        for(int j = i + 1; j >= 1; j--){
            dp[j] = dp[j] * (1.0 - p[i]) + dp[j-1] * p[i];
        }
        dp[0] *= (1.0 - p[i]);
    }

    double ans = 0;
    for(int j = N / 2 + 1; j <= N; j++) ans += dp[j];
    printf("%.10f\n", ans);
}
