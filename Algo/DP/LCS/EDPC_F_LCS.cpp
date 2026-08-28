// 最長共通部分列 (LCS: Longest Common Subsequence)
// 例題: EDPC F - LCS
// https://atcoder.jp/contests/dp/tasks/dp_f
// 計算量: O(NM)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: LCS長 ===
int lcs_length(const string& s, const string& t){
    int n = s.size(), m = t.size();
    // dp[j] = s[0..i) と t[0..j) のLCS長 (1次元圧縮)
    vector<int> dp(m + 1, 0);
    for(int i = 0; i < n; i++){
        int prev = 0;
        for(int j = 0; j < m; j++){
            int tmp = dp[j + 1];
            if(s[i] == t[j]) dp[j + 1] = prev + 1;
            else dp[j + 1] = max(dp[j + 1], dp[j]);
            prev = tmp;
        }
    }
    return dp[m];
}

// === ライブラリ: LCS復元 ===
string lcs_restore(const string& s, const string& t){
    int n = s.size(), m = t.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            if(s[i] == t[j]) dp[i+1][j+1] = dp[i][j] + 1;
            else dp[i+1][j+1] = max(dp[i+1][j], dp[i][j+1]);
        }
    }
    // 復元
    string res;
    int i = n, j = m;
    while(i > 0 && j > 0){
        if(s[i-1] == t[j-1]){
            res += s[i-1];
            i--; j--;
        } else if(dp[i-1][j] > dp[i][j-1]){
            i--;
        } else {
            j--;
        }
    }
    reverse(res.begin(), res.end());
    return res;
}

// === 例題: EDPC F ===
int main(){
    string s, t;
    cin >> s >> t;
    cout << lcs_restore(s, t) << endl;
    return 0;
}
