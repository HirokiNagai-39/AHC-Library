// Palindrome Partition
// Minimum number of cuts to partition string into palindromes. O(N^2)
// dp[i] = min cuts for s[0..i]. O(N^2)
#include <bits/stdc++.h>
using namespace std;
int main(){
    string s; cin>>s;
    int n=s.size();
    // is_pal[i][j] = true if s[i..j] is palindrome
    vector<vector<bool>> is_pal(n,vector<bool>(n,false));
    for(int i=n-1;i>=0;i--){
        for(int j=i;j<n;j++){
            if(s[i]==s[j]&&(j-i<=2||is_pal[i+1][j-1]))
                is_pal[i][j]=true;
        }
    }
    // dp[i] = min cuts to partition s[0..i] into palindromes
    vector<int> dp(n,n);
    for(int i=0;i<n;i++){
        if(is_pal[0][i]){ dp[i]=0; continue; }
        for(int j=1;j<=i;j++){
            if(is_pal[j][i])
                dp[i]=min(dp[i],dp[j-1]+1);
        }
    }
    cout<<dp[n-1]<<endl;
}
