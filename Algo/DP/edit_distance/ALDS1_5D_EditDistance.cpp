// ALDS1 - Edit Distance (basic)
// Standard edit distance (Levenshtein distance) between two strings.
// Operations: insert, delete, replace. Each costs 1. O(NM)
#include <bits/stdc++.h>
using namespace std;

int main(){
    string S, T;
    cin >> S >> T;
    int n = S.size(), m = T.size();

    vector<int> dp(m + 1);
    for(int j = 0; j <= m; j++) dp[j] = j;

    for(int i = 1; i <= n; i++){
        vector<int> ndp(m + 1);
        ndp[0] = i;
        for(int j = 1; j <= m; j++){
            if(S[i-1] == T[j-1]){
                ndp[j] = dp[j-1];
            } else {
                ndp[j] = min({dp[j-1], dp[j], ndp[j-1]}) + 1;
            }
        }
        swap(dp, ndp);
    }
    cout << dp[m] << endl;
}
