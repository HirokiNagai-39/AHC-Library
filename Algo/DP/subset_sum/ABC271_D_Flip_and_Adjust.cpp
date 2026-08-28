// ABC271 D - Flip and Adjust
// https://atcoder.jp/contests/abc271/tasks/abc271_d
// N cards with front/back values. Choose a side for each card so sum = S.
// Subset sum DP with path reconstruction. O(NS)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N, S;
    cin >> N >> S;
    vector<int> a(N), b(N);
    for(int i = 0; i < N; i++) cin >> a[i] >> b[i];

    // dp[j] = true if sum j is achievable
    // choice[i][j] = 'H' or 'T' (which side was chosen at card i to reach sum j)
    vector<vector<bool>> dp(N + 1, vector<bool>(S + 1, false));
    vector<vector<char>> choice(N, vector<char>(S + 1, '?'));
    dp[0][0] = true;

    for(int i = 0; i < N; i++){
        for(int j = 0; j <= S; j++){
            if(!dp[i][j]) continue;
            if(j + a[i] <= S && !dp[i+1][j + a[i]]){
                dp[i+1][j + a[i]] = true;
                choice[i][j + a[i]] = 'H';
            }
            if(j + b[i] <= S && !dp[i+1][j + b[i]]){
                dp[i+1][j + b[i]] = true;
                choice[i][j + b[i]] = 'T';
            }
        }
    }

    if(!dp[N][S]){
        cout << "No" << endl;
        return 0;
    }
    cout << "Yes" << endl;
    string ans(N, '?');
    int cur = S;
    for(int i = N - 1; i >= 0; i--){
        ans[i] = choice[i][cur];
        if(ans[i] == 'H') cur -= a[i];
        else cur -= b[i];
    }
    cout << ans << endl;
}
