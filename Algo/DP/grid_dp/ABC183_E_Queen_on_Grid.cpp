// ABC183 E - Queen on Grid
// https://atcoder.jp/contests/abc183/tasks/abc183_e
// H*W grid, queen moves (right/down/diagonal). Count paths (1,1)->(H,W).
// Prefix sum DP to avoid O(HW*max(H,W)). O(HW)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1000000007;

int main(){
    int H, W;
    cin >> H >> W;
    vector<string> grid(H);
    for(auto &s : grid) cin >> s;

    vector<vector<long long>> dp(H, vector<long long>(W, 0));
    // sr: cumulative sum from left (right direction)
    // sd: cumulative sum from top (down direction)
    // srd: cumulative sum from top-left (diagonal direction)
    vector<vector<long long>> sr(H, vector<long long>(W, 0));
    vector<vector<long long>> sd(H, vector<long long>(W, 0));
    vector<vector<long long>> srd(H, vector<long long>(W, 0));

    dp[0][0] = 1;
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            if(grid[i][j] == '#'){
                sr[i][j] = sd[i][j] = srd[i][j] = 0;
                continue;
            }
            if(i > 0) dp[i][j] = (dp[i][j] + sd[i-1][j]) % MOD;
            if(j > 0) dp[i][j] = (dp[i][j] + sr[i][j-1]) % MOD;
            if(i > 0 && j > 0) dp[i][j] = (dp[i][j] + srd[i-1][j-1]) % MOD;

            sr[i][j] = ((j > 0 ? sr[i][j-1] : 0) + dp[i][j]) % MOD;
            sd[i][j] = ((i > 0 ? sd[i-1][j] : 0) + dp[i][j]) % MOD;
            srd[i][j] = ((i > 0 && j > 0 ? srd[i-1][j-1] : 0) + dp[i][j]) % MOD;
        }
    }
    cout << dp[H-1][W-1] << endl;
}
