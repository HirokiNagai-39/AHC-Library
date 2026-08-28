// Grid Path Max Sum (original example)
// H*W grid with values, move right or down from (1,1) to (H,W).
// Maximize sum of values along the path. O(HW)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int H, W;
    cin >> H >> W;
    vector<vector<long long>> a(H, vector<long long>(W));
    for(int i = 0; i < H; i++)
        for(int j = 0; j < W; j++)
            cin >> a[i][j];

    vector<vector<long long>> dp(H, vector<long long>(W, -1e18));
    dp[0][0] = a[0][0];
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            if(i == 0 && j == 0) continue;
            if(i > 0) dp[i][j] = max(dp[i][j], dp[i-1][j] + a[i][j]);
            if(j > 0) dp[i][j] = max(dp[i][j], dp[i][j-1] + a[i][j]);
        }
    }
    cout << dp[H-1][W-1] << endl;
}
