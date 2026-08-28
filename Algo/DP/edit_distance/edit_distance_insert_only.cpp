// Edit Distance (Insert Only)
// Only insertions allowed to transform S into T.
// Minimum insertions = |T| - LCS(S, T). O(NM)
#include <bits/stdc++.h>
using namespace std;

int main(){
    string S, T;
    cin >> S >> T;
    int n = S.size(), m = T.size();

    // Compute LCS length
    vector<int> dp(m + 1, 0);
    for(int i = 1; i <= n; i++){
        vector<int> ndp(m + 1, 0);
        for(int j = 1; j <= m; j++){
            if(S[i-1] == T[j-1]) ndp[j] = dp[j-1] + 1;
            else ndp[j] = max(ndp[j-1], dp[j]);
        }
        swap(dp, ndp);
    }
    int lcs = dp[m];
    // Characters of S not in LCS must be deleted (but we can only insert).
    // So S must be a subsequence of T for a solution to exist with insert only.
    // If S is a subsequence of T: answer = |T| - |S|
    // General case (insert + delete): |S| - lcs deletions + |T| - lcs insertions
    // Insert-only from S to T is only possible if S is subseq of T.
    // Here we output the general formula: min ops = |S| + |T| - 2*lcs
    // where deletions and insertions each cost 1.
    // Pure insert-only: answer = |T| - lcs (if we allow reordering, otherwise check subseq)
    cout << "LCS length: " << lcs << endl;
    cout << "Min insertions (into S to contain T as subseq): " << m - lcs << endl;
    cout << "Min deletions (from S): " << n - lcs << endl;
    cout << "Min edit (insert+delete only): " << (n - lcs) + (m - lcs) << endl;
}
