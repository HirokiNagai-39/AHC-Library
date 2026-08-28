// 編集距離 (Edit Distance / Levenshtein Distance)
// 文字列sをtに変換する最小操作回数 (挿入/削除/置換)
// 例題: 典型問題, DPL_1_E (Aizu)
// 計算量: O(NM)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 編集距離 ===
int edit_distance(const string& s, const string& t){
    int n = s.size(), m = t.size();
    // dp[i][j] = s[0..i) → t[0..j) の編集距離
    // 1次元圧縮
    vector<int> dp(m + 1);
    for(int j = 0; j <= m; j++) dp[j] = j; // s="" → t[0..j) は j回挿入

    for(int i = 0; i < n; i++){
        int prev = dp[0];
        dp[0] = i + 1; // s[0..i+1) → t="" は i+1回削除
        for(int j = 0; j < m; j++){
            int tmp = dp[j + 1];
            if(s[i] == t[j]){
                dp[j + 1] = prev;
            } else {
                dp[j + 1] = min({prev,      // 置換
                                 dp[j],      // 挿入
                                 dp[j + 1]   // 削除
                                }) + 1;
            }
            prev = tmp;
        }
    }
    return dp[m];
}

// === ライブラリ: 編集距離 (復元あり, 2次元版) ===
pair<int, vector<string>> edit_distance_with_ops(const string& s, const string& t){
    int n = s.size(), m = t.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1));
    for(int i = 0; i <= n; i++) dp[i][0] = i;
    for(int j = 0; j <= m; j++) dp[0][j] = j;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            if(s[i] == t[j]) dp[i+1][j+1] = dp[i][j];
            else dp[i+1][j+1] = min({dp[i][j], dp[i+1][j], dp[i][j+1]}) + 1;
        }
    }
    // 復元
    vector<string> ops;
    int i = n, j = m;
    while(i > 0 || j > 0){
        if(i > 0 && j > 0 && s[i-1] == t[j-1]){
            i--; j--;
        } else if(i > 0 && j > 0 && dp[i][j] == dp[i-1][j-1] + 1){
            ops.push_back("replace s[" + to_string(i-1) + "] " +
                          string(1, s[i-1]) + " -> " + string(1, t[j-1]));
            i--; j--;
        } else if(j > 0 && dp[i][j] == dp[i][j-1] + 1){
            ops.push_back("insert " + string(1, t[j-1]) + " at " + to_string(i));
            j--;
        } else {
            ops.push_back("delete s[" + to_string(i-1) + "] = " + string(1, s[i-1]));
            i--;
        }
    }
    reverse(ops.begin(), ops.end());
    return {dp[n][m], ops};
}

// === 例題 ===
int main(){
    string s, t;
    cin >> s >> t;
    cout << edit_distance(s, t) << endl;
    return 0;
}
