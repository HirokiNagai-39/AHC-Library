// ABC179 D - Leaping Tak
// https://atcoder.jp/contests/abc179/tasks/abc179_d
// Jump from 1 to N, distance in union of intervals [L_i, R_i].
// dp[i] = ways to reach i. Use prefix sum for range updates. O(NK)
#include <bits/stdc++.h>
using namespace std;
const long long MOD=998244353;
int main(){
    int N,K; cin>>N>>K;
    vector<int> L(K),R(K);
    for(int i=0;i<K;i++) cin>>L[i]>>R[i];
    vector<long long> dp(N+1,0);
    dp[1]=1;
    // prefix sum of dp
    vector<long long> psum(N+2,0);
    psum[1]=1; psum[2]=psum[1]; // psum[i] = sum of dp[0..i-1]
    // Actually maintain running prefix sum
    vector<long long> S(N+2,0); // S[i] = dp[1]+dp[2]+...+dp[i]
    S[1]=1;
    for(int i=2;i<=N;i++){
        for(int k=0;k<K;k++){
            int lo=i-R[k], hi=i-L[k];
            if(hi<1) continue;
            lo=max(lo,1);
            // dp[i] += S[hi] - S[lo-1]
            dp[i]=(dp[i]+S[hi]-S[lo-1]+MOD)%MOD;
        }
        S[i]=(S[i-1]+dp[i])%MOD;
    }
    cout<<dp[N]<<endl;
}
