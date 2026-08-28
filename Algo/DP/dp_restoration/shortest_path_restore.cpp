// Shortest Path Restoration (DP on DAG / Grid)
// Grid shortest path from (0,0) to (H-1,W-1), moving right or down.
// Restore the actual path. O(HW)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int H,W; cin>>H>>W;
    vector<vector<int>> grid(H,vector<int>(W));
    for(int i=0;i<H;i++) for(int j=0;j<W;j++) cin>>grid[i][j];
    const int INF=1e9;
    vector<vector<int>> dp(H,vector<int>(W,INF));
    dp[0][0]=grid[0][0];
    for(int i=0;i<H;i++) for(int j=0;j<W;j++){
        if(i==0&&j==0) continue;
        if(i>0) dp[i][j]=min(dp[i][j],dp[i-1][j]+grid[i][j]);
        if(j>0) dp[i][j]=min(dp[i][j],dp[i][j-1]+grid[i][j]);
    }
    cout<<dp[H-1][W-1]<<endl;
    // Restore path
    string path;
    int r=H-1,c=W-1;
    while(r>0||c>0){
        if(r==0){ path+='R'; c--; }
        else if(c==0){ path+='D'; r--; }
        else if(dp[r-1][c]<dp[r][c-1]){ path+='D'; r--; }
        else { path+='R'; c--; }
    }
    reverse(path.begin(),path.end());
    cout<<path<<endl;
}
