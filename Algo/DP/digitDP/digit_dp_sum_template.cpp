// Digit DP Template - Count numbers in [1,N] with digit sum divisible by M
// Generic template applicable to many digit DP problems.
#include <bits/stdc++.h>
using namespace std;
int main(){
    string N; int M;
    cin>>N>>M;
    int n=N.size();
    // dp[pos][sum_mod][tight]
    vector<vector<vector<long long>>> memo(n,vector<vector<long long>>(M,vector<long long>(2,-1)));
    function<long long(int,int,bool)> dp=[&](int pos,int smod,bool tight)->long long{
        if(pos==n) return smod==0?1:0;
        if(memo[pos][smod][tight]!=-1) return memo[pos][smod][tight];
        int lim=tight?(N[pos]-'0'):9;
        long long res=0;
        for(int d=0;d<=lim;d++){
            res+=dp(pos+1,(smod+d)%M,tight&&(d==lim));
        }
        return memo[pos][smod][tight]=res;
    };
    long long ans=dp(0,0,true)-1; // subtract 0
    cout<<ans<<endl;
}
