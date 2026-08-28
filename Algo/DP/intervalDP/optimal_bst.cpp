// Optimal Binary Search Tree
// Given keys with access frequencies, find BST minimizing total search cost.
// dp[i][j] = min cost for keys i..j. O(N^3)
// Knuth's optimization can make it O(N^2) but O(N^3) is shown here.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int n; cin>>n;
    vector<double> freq(n);
    for(int i=0;i<n;i++) cin>>freq[i];
    // prefix sum of frequencies
    vector<double> psum(n+1,0);
    for(int i=0;i<n;i++) psum[i+1]=psum[i]+freq[i];
    auto sum=[&](int l,int r)->double{ return psum[r+1]-psum[l]; };
    // dp[i][j] = min expected search cost for keys i..j
    vector<vector<double>> dp(n,vector<double>(n,1e18));
    for(int i=0;i<n;i++) dp[i][i]=freq[i];
    for(int len=2;len<=n;len++){
        for(int i=0;i+len-1<n;i++){
            int j=i+len-1;
            for(int r=i;r<=j;r++){
                double cost=sum(i,j);
                if(r>i) cost+=dp[i][r-1];
                if(r<j) cost+=dp[r+1][j];
                dp[i][j]=min(dp[i][j],cost);
            }
        }
    }
    printf("%.6f\n",dp[0][n-1]);
}
