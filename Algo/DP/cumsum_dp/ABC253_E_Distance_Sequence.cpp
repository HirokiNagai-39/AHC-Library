// ABC253 E - Distance Sequence
// https://atcoder.jp/contests/abc253/tasks/abc253_e
// Count length-N sequences with values in [1,M], |a_i - a_{i+1}| >= K.
// dp[i][v] = ways ending with value v at position i. Prefix sum optimization. O(NM)
#include <bits/stdc++.h>
using namespace std;
const long long MOD=998244353;
int main(){
    int N,M,K; cin>>N>>M>>K;
    vector<long long> dp(M+1,1); // dp[v] for current position, all 1 initially
    // prefix sum: S[v] = dp[1]+...+dp[v]
    for(int i=1;i<N;i++){
        vector<long long> ndp(M+1,0);
        // build prefix sum of dp
        vector<long long> S(M+2,0);
        for(int v=1;v<=M;v++) S[v]=(S[v-1]+dp[v])%MOD;
        if(K==0){
            // no constraint: ndp[v] = S[M] for all v
            for(int v=1;v<=M;v++) ndp[v]=S[M];
        } else {
            for(int v=1;v<=M;v++){
                // allowed: values u with |u-v|>=K, i.e., u<=v-K or u>=v+K
                long long sum=0;
                if(v-K>=1) sum=(sum+S[v-K])%MOD;
                if(v+K<=M) sum=(sum+S[M]-S[v+K-1]+MOD)%MOD;
                ndp[v]=sum;
            }
        }
        dp=ndp;
    }
    long long ans=0;
    for(int v=1;v<=M;v++) ans=(ans+dp[v])%MOD;
    cout<<ans<<endl;
}
