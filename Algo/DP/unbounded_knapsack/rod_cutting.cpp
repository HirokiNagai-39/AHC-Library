// Rod Cutting Problem
// Rod of length N. Price p[i] for piece of length i. Maximize revenue. O(N^2)
// Unbounded knapsack variant: dp[j] = max revenue for rod of length j.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<int> p(N+1);
    for(int i=1;i<=N;i++) cin>>p[i]; // price for length i
    vector<int> dp(N+1,0);
    for(int j=1;j<=N;j++){
        for(int i=1;i<=j;i++){
            dp[j]=max(dp[j],dp[j-i]+p[i]);
        }
    }
    cout<<dp[N]<<endl;
    // Restoration: which cuts were made
    vector<int> cut(N+1,0);
    // recompute with tracking
    fill(dp.begin(),dp.end(),0);
    for(int j=1;j<=N;j++){
        for(int i=1;i<=j;i++){
            if(dp[j-i]+p[i]>dp[j]){
                dp[j]=dp[j-i]+p[i];
                cut[j]=i;
            }
        }
    }
    // print pieces
    int rem=N;
    while(rem>0){
        cout<<cut[rem]<<" ";
        rem-=cut[rem];
    }
    cout<<endl;
}
