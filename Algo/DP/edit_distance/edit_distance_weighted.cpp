// Weighted Edit Distance
// Each operation (insert, delete, replace) has a different cost.
// Generalized Levenshtein distance. O(NM)
#include <bits/stdc++.h>
using namespace std;

int main(){
    // Cost parameters
    int cost_insert, cost_delete, cost_replace;
    cin >> cost_insert >> cost_delete >> cost_replace;

    string S, T;
    cin >> S >> T;
    int n = S.size(), m = T.size();

    vector<vector<long long>> dp(n + 1, vector<long long>(m + 1, 0));
    for(int i = 1; i <= n; i++) dp[i][0] = (long long)i * cost_delete;
    for(int j = 1; j <= m; j++) dp[0][j] = (long long)j * cost_insert;

    for(int i = 1; i <= n; i++){
        for(int j = 1; j <= m; j++){
            if(S[i-1] == T[j-1]){
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = dp[i-1][j-1] + cost_replace;
            }
            dp[i][j] = min(dp[i][j], dp[i-1][j] + cost_delete);
            dp[i][j] = min(dp[i][j], dp[i][j-1] + cost_insert);
        }
    }
    cout << dp[n][m] << endl;
}
