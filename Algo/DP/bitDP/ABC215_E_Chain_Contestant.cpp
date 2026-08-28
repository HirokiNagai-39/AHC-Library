// ABC215 E - Chain Contestant
// https://atcoder.jp/contests/abc215/tasks/abc215_e
// Choose subsequence where same chars are contiguous.
// dp[mask][last] = ways using char set "mask" with last char "last". O(N*26*2^10)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    string S; cin>>S;
    const long long MOD=998244353;
    // 10 distinct characters A-J
    int C=10;
    int full=1<<C;
    // dp[mask][last]: number of subsequences using exactly chars in mask, ending with char last
    vector<vector<long long>> dp(full,vector<long long>(C,0));
    for(int i=0;i<N;i++){
        int c=S[i]-'A';
        // new subsequences: just this character alone
        dp[1<<c][c]=(dp[1<<c][c]+1)%MOD;
        // extend existing subsequences
        for(int mask=0;mask<full;mask++){
            for(int last=0;last<C;last++){
                if(dp[mask][last]==0) continue;
                if(last==c){
                    // extend with same char
                    dp[mask][c]=(dp[mask][c]+dp[mask][last])%MOD;
                } else if(!(mask>>c&1)){
                    // extend with new char
                    int nmask=mask|(1<<c);
                    dp[nmask][c]=(dp[nmask][c]+dp[mask][last])%MOD;
                }
            }
        }
    }
    long long ans=0;
    for(int mask=0;mask<full;mask++)
        for(int c=0;c<C;c++)
            ans=(ans+dp[mask][c])%MOD;
    cout<<ans<<endl;
}
