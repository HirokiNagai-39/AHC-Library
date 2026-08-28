// Matrix Chain Multiplication (ALDS1_10_B equivalent)
// https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_10_B
// Find minimum scalar multiplications for matrix chain A1*A2*...*An.
// dp[i][j] = min cost to multiply matrices i..j. O(N^3)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int n; cin>>n;
    vector<int> r(n),c(n);
    for(int i=0;i<n;i++) cin>>r[i]>>c[i];
    // dp[i][j] = min cost for multiplying matrices i..j
    vector<vector<long long>> dp(n,vector<long long>(n,0));
    for(int len=2;len<=n;len++){
        for(int i=0;i+len-1<n;i++){
            int j=i+len-1;
            dp[i][j]=1e18;
            for(int k=i;k<j;k++){
                long long cost=dp[i][k]+dp[k+1][j]+(long long)r[i]*c[k]*c[j];
                dp[i][j]=min(dp[i][j],cost);
            }
        }
    }
    cout<<dp[0][n-1]<<endl;
}
