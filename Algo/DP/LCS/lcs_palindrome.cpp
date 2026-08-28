// Longest Palindromic Subsequence
// = LCS of string S and reverse(S). O(N^2)
#include <bits/stdc++.h>
using namespace std;

int main(){
    string S;
    cin >> S;
    int n = S.size();
    string T = S;
    reverse(T.begin(), T.end());

    // Space-optimized LCS
    vector<int> dp(n + 1, 0), ndp(n + 1, 0);
    for(int i = 1; i <= n; i++){
        fill(ndp.begin(), ndp.end(), 0);
        for(int j = 1; j <= n; j++){
            if(S[i-1] == T[j-1]) ndp[j] = dp[j-1] + 1;
            else ndp[j] = max(ndp[j-1], dp[j]);
        }
        swap(dp, ndp);
    }
    cout << dp[n] << endl;
}
