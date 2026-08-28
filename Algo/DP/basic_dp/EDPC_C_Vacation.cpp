// EDPC C - Vacation
// https://atcoder.jp/contests/dp/tasks/dp_c
// N days, 3 activities per day (no consecutive same activity), maximize happiness.
// dp[i][j] = max happiness on day i choosing activity j. O(N)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int n;
    cin >> n;
    vector<array<long long,3>> a(n);
    for(int i = 0; i < n; i++)
        cin >> a[i][0] >> a[i][1] >> a[i][2];

    vector<array<long long,3>> dp(n);
    dp[0] = a[0];
    for(int i = 1; i < n; i++){
        for(int j = 0; j < 3; j++){
            for(int k = 0; k < 3; k++){
                if(j == k) continue;
                dp[i][j] = max(dp[i][j], dp[i-1][k] + a[i][j]);
            }
        }
    }
    cout << *max_element(dp[n-1].begin(), dp[n-1].end()) << endl;
}
