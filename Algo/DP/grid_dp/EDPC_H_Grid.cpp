// グリッドDP: EDPC H - Grid 1
// https://atcoder.jp/contests/dp/tasks/dp_h
// H*Wグリッドの左上→右下への経路数 (障害物あり)
// 計算量: O(HW)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

int main(){
    int H, W;
    cin >> H >> W;
    vector<string> grid(H);
    for(int i = 0; i < H; i++) cin >> grid[i];

    vector<vector<long long>> dp(H, vector<long long>(W, 0));
    dp[0][0] = (grid[0][0] == '.') ? 1 : 0;

    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            if(grid[i][j] == '#') continue;
            if(i > 0) dp[i][j] = (dp[i][j] + dp[i-1][j]) % MOD;
            if(j > 0) dp[i][j] = (dp[i][j] + dp[i][j-1]) % MOD;
        }
    }
    cout << dp[H-1][W-1] << endl;
    return 0;
}
