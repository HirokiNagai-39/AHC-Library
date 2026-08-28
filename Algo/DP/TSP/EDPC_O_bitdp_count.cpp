// bitDP: 巡回順の数え上げ
// N個の仕事をN人に割り当てる方法数 (EDPC Oと同型)
// 計算量: O(N * 2^N)
#include <bits/stdc++.h>
using namespace std;
static constexpr long long MOD=1e9+7;
int main(){
    int N; cin>>N;
    vector<vector<int>> a(N,vector<int>(N));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) cin>>a[i][j];
    vector<long long> dp(1<<N,0);
    dp[0]=1;
    for(int S=0;S<(1<<N);S++){
        int i=__builtin_popcount(S);
        if(i>=N) continue;
        for(int j=0;j<N;j++){
            if(S>>j&1) continue;
            if(!a[i][j]) continue;
            dp[S|(1<<j)]=(dp[S|(1<<j)]+dp[S])%MOD;
        }
    }
    cout<<dp[(1<<N)-1]<<endl;
}
