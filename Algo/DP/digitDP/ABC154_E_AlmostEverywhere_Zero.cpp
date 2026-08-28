// ABC154 E - Almost Everywhere Zero
// https://atcoder.jp/contests/abc154/tasks/abc154_e
// Count numbers in [1,N] with exactly K non-zero digits. Digit DP.
#include <bits/stdc++.h>
using namespace std;
int main(){
    string N; int K;
    cin>>N>>K;
    int n=N.size();
    // dp[pos][cnt][tight]
    // memo[pos][cnt][tight]
    vector<vector<vector<long long>>> dp(n+1,vector<vector<long long>>(K+1,vector<long long>(2,-1)));
    function<long long(int,int,bool)> solve=[&](int pos,int cnt,bool tight)->long long{
        if(cnt>K) return 0;
        if(pos==n) return cnt==K?1:0;
        if(dp[pos][cnt][tight]!=-1) return dp[pos][cnt][tight];
        int lim=tight?(N[pos]-'0'):9;
        long long res=0;
        for(int d=0;d<=lim;d++){
            res+=solve(pos+1,cnt+(d!=0),tight&&(d==lim));
        }
        return dp[pos][cnt][tight]=res;
    };
    cout<<solve(0,0,true)<<endl;
}
