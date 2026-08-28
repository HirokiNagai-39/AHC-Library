// Longest Common Substring (contiguous)
// Given strings S and T, find the length of the longest common contiguous substring. O(NM)
#include <bits/stdc++.h>
using namespace std;

int main(){
    string S, T;
    cin >> S >> T;
    int n = S.size(), m = T.size();

    // dp[j] = length of common substring ending at S[i-1] and T[j-1]
    vector<int> dp(m + 1, 0), ndp(m + 1, 0);
    int ans = 0;

    for(int i = 1; i <= n; i++){
        fill(ndp.begin(), ndp.end(), 0);
        for(int j = 1; j <= m; j++){
            if(S[i-1] == T[j-1]){
                ndp[j] = dp[j-1] + 1;
                ans = max(ans, ndp[j]);
            }
        }
        swap(dp, ndp);
    }
    cout << ans << endl;
}
