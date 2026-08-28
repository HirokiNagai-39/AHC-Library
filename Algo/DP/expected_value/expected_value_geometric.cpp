// Expected Value - Geometric Distribution and Coupon Collector
// Geometric: expected trials for success with probability p = 1/p
// Coupon collector: expected draws to collect all N types = N * H(N)
// where H(N) = 1 + 1/2 + ... + 1/N (harmonic number)
// Example problem: N types of stickers, each equally likely. Expected draws for complete set.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    // Coupon collector's problem
    // After collecting k types, prob of new type = (N-k)/N
    // Expected draws for next new type = N/(N-k)
    // Total = sum_{k=0}^{N-1} N/(N-k) = N * sum_{i=1}^{N} 1/i
    double ans=0;
    for(int k=0;k<N;k++){
        ans+=(double)N/(N-k);
    }
    printf("%.9f\n",ans);
}
// Note: For modular arithmetic version, compute sum of N * inv(i) for i=1..N mod P.
// Example with mod:
// const long long MOD = 1e9+7;
// long long pw(long long a,long long b,long long m){...}
// ans = sum of N * pw(i, MOD-2, MOD) for i=1..N, all mod MOD.
