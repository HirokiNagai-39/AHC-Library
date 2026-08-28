// Diff using LCS
// Given two strings (or line sequences), output a diff showing additions
// and deletions, computed via LCS backtracking. O(NM)
#include <bits/stdc++.h>
using namespace std;

int main(){
    string S, T;
    cin >> S >> T;
    int n = S.size(), m = T.size();

    vector<vector<int>> dp(n+1, vector<int>(m+1, 0));
    for(int i = 1; i <= n; i++)
        for(int j = 1; j <= m; j++){
            if(S[i-1] == T[j-1]) dp[i][j] = dp[i-1][j-1] + 1;
            else dp[i][j] = max(dp[i-1][j], dp[i][j-1]);
        }

    // Backtrack to produce diff
    vector<pair<char,char>> diff; // (type, char): ' '=same, '-'=delete, '+'=insert
    int i = n, j = m;
    while(i > 0 || j > 0){
        if(i > 0 && j > 0 && S[i-1] == T[j-1]){
            diff.push_back({' ', S[i-1]});
            i--; j--;
        } else if(i > 0 && (j == 0 || dp[i-1][j] >= dp[i][j-1])){
            diff.push_back({'-', S[i-1]});
            i--;
        } else {
            diff.push_back({'+', T[j-1]});
            j--;
        }
    }
    reverse(diff.begin(), diff.end());
    for(auto &[type, c] : diff)
        cout << type << c << "\n";
}
