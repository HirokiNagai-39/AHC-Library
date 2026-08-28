// Stone Game - Take 1 to K stones, last to take wins
// dp[i] = true if current player wins with i stones remaining.
// dp[0] = false (no stones = lose). dp[i] = OR(NOT dp[i-j]) for j=1..K. O(NK)
// Pattern: dp[i] = (i % (K+1) != 0) for standard Nim with 1..K removal.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,K; cin>>N>>K;
    // Method 1: Direct formula
    // cout << (N%(K+1)!=0 ? "First" : "Second") << endl;
    // Method 2: DP (works for arbitrary removal sets too)
    vector<bool> dp(N+1,false);
    // dp[0]=false (losing position)
    for(int i=1;i<=N;i++){
        for(int j=1;j<=K&&j<=i;j++){
            if(!dp[i-j]){ dp[i]=true; break; }
        }
    }
    cout<<(dp[N]?"First":"Second")<<endl;
}
