// ABC185 E - Sequence Matching
// https://atcoder.jp/contests/abc185/tasks/abc185_e
// Two integer sequences A, B. Delete elements to make them equal.
// Cost = (deleted elements) + (mismatched positions). Minimize cost. O(NM)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N, M;
    cin >> N >> M;
    vector<int> A(N), B(M);
    for(auto &x : A) cin >> x;
    for(auto &x : B) cin >> x;

    // dp[j] = min cost matching A[0..i-1] with B[0..j-1]
    vector<int> dp(M + 1);
    for(int j = 0; j <= M; j++) dp[j] = j; // delete all of B[0..j-1]

    for(int i = 1; i <= N; i++){
        vector<int> ndp(M + 1);
        ndp[0] = i; // delete all of A[0..i-1]
        for(int j = 1; j <= M; j++){
            if(A[i-1] == B[j-1]){
                ndp[j] = dp[j-1]; // match, no cost
            } else {
                ndp[j] = dp[j-1] + 1; // mismatch cost
            }
            ndp[j] = min(ndp[j], min(dp[j], ndp[j-1]) + 1); // delete from A or B
        }
        swap(dp, ndp);
    }
    cout << dp[M] << endl;
}
