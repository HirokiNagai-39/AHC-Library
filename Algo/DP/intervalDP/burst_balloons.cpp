// Burst Balloons (LeetCode 312 equivalent)
// N balloons with values. Bursting balloon i gives nums[l]*nums[i]*nums[r].
// Find max coins by bursting all. Interval DP. O(N^3)
// dp[l][r] = max coins from bursting all balloons in (l,r) exclusive.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int n; cin>>n;
    vector<int> nums(n);
    for(int i=0;i<n;i++) cin>>nums[i];
    // add boundary 1s
    vector<int> a={1};
    for(int x:nums) a.push_back(x);
    a.push_back(1);
    int m=a.size();
    // dp[i][j] = max coins from bursting all balloons strictly between i and j
    vector<vector<long long>> dp(m,vector<long long>(m,0));
    for(int len=2;len<m;len++){
        for(int i=0;i+len<m;i++){
            int j=i+len;
            for(int k=i+1;k<j;k++){
                long long val=dp[i][k]+dp[k][j]+(long long)a[i]*a[k]*a[j];
                dp[i][j]=max(dp[i][j],val);
            }
        }
    }
    cout<<dp[0][m-1]<<endl;
}
