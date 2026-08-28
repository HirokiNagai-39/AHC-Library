// ABC175 E - Picking Goods
// https://atcoder.jp/contests/abc175/tasks/abc175_e
// R*C grid, items placed on cells. Move right or down from (1,1) to (R,C).
// At most 3 items per row. dp[i][j][k] = max value at (i,j) having picked k items in row i. O(RC)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int R, C, K;
    cin >> R >> C >> K;
    // value at each cell
    vector<vector<long long>> v(R + 1, vector<long long>(C + 1, 0));
    for(int i = 0; i < K; i++){
        int r, c; long long val;
        cin >> r >> c >> val;
        v[r][c] = val;
    }

    // dp[j][k] = max value at column j having picked k items in current row
    // Process row by row
    static constexpr long long NEG = -1e18;
    vector<vector<long long>> dp(C + 1, vector<long long>(4, NEG));
    vector<vector<long long>> ndp(C + 1, vector<long long>(4, NEG));
    dp[1][0] = 0;
    if(v[1][1] > 0) dp[1][1] = v[1][1];

    for(int i = 1; i <= R; i++){
        if(i > 1){
            // Transition from previous row: take max over all k, reset k to 0 or 1
            for(int j = 1; j <= C; j++){
                for(int k = 0; k < 4; k++) ndp[j][k] = NEG;
            }
            for(int j = 1; j <= C; j++){
                long long best = NEG;
                for(int k = 0; k < 4; k++) best = max(best, dp[j][k]);
                if(best == NEG) continue;
                ndp[j][0] = max(ndp[j][0], best);
                if(v[i][j] > 0)
                    ndp[j][1] = max(ndp[j][1], best + v[i][j]);
            }
            swap(dp, ndp);
            // Now process row i left to right
        }
        for(int j = 2; j <= C; j++){
            for(int k = 0; k < 4; k++){
                if(dp[j-1][k] == NEG) continue;
                dp[j][k] = max(dp[j][k], dp[j-1][k]);
                if(k + 1 < 4 && v[i][j] > 0)
                    dp[j][k+1] = max(dp[j][k+1], dp[j-1][k] + v[i][j]);
            }
        }
    }

    long long ans = 0;
    for(int k = 0; k < 4; k++)
        if(dp[C][k] != NEG) ans = max(ans, dp[C][k]);
    cout << ans << endl;
}
