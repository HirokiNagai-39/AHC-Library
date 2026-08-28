// LCS of 3 strings
// Given strings S, T, U, find the length of their longest common subsequence. O(NML)
#include <bits/stdc++.h>
using namespace std;

int main(){
    string S, T, U;
    cin >> S >> T >> U;
    int n = S.size(), m = T.size(), l = U.size();

    vector<vector<vector<int>>> dp(n+1, vector<vector<int>>(m+1, vector<int>(l+1, 0)));
    for(int i = 1; i <= n; i++){
        for(int j = 1; j <= m; j++){
            for(int k = 1; k <= l; k++){
                if(S[i-1] == T[j-1] && T[j-1] == U[k-1]){
                    dp[i][j][k] = dp[i-1][j-1][k-1] + 1;
                } else {
                    dp[i][j][k] = max({dp[i-1][j][k], dp[i][j-1][k], dp[i][j][k-1]});
                }
            }
        }
    }
    cout << dp[n][m][l] << endl;
}
