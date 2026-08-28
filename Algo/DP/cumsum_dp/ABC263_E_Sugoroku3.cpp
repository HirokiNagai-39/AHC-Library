// ABC263 E - Sugoroku 3
// https://atcoder.jp/contests/abc263/tasks/abc263_e
// Board game: at cell i, roll dice landing on i+1..i+A[i] uniformly.
// Find expected moves from 1 to N. Backward DP with prefix sum. O(N)
// E[i] = 1 + (1/A[i]) * sum(E[j] for j=i+1..i+A[i])
// Rearranging: E[i] = (A[i] + sum(E[j])) / A[i]
// But if some cells j have A[j]=0 (teleport to 0)... actually problem guarantees reachability.
// Actually: at cell i, move to i+1,...,i+A[i] each with prob 1/A[i].
#include <bits/stdc++.h>
using namespace std;
const long long MOD=998244353;
long long pw(long long a,long long b,long long m){long long r=1;a%=m;while(b>0){if(b&1)r=r*a%m;a=a*a%m;b>>=1;}return r;}
long long inv(long long a){return pw(a,MOD-2,MOD);}
int main(){
    int N; cin>>N;
    vector<int> A(N);
    for(int i=0;i<N-1;i++) cin>>A[i];
    // E[N-1]=0 (0-indexed, target is N-1)
    vector<long long> E(N,0);
    long long suffix=0; // suffix sum of E[i+1..i+A[i]]
    // Process from N-2 down to 0
    // Maintain suffix sum S[i] = E[i] + E[i+1] + ... + E[N-1]
    vector<long long> S(N+1,0);
    for(int i=N-2;i>=0;i--){
        // E[i] = (A[i] + S[i+1] - S[i+1+A[i]]) * inv(A[i])
        long long sum_range=(S[i+1]-S[min(i+1+A[i],N)]+MOD)%MOD;
        E[i]=(A[i]+sum_range)%MOD*inv(A[i])%MOD;
        S[i]=(S[i+1]+E[i])%MOD;
    }
    cout<<E[0]<<endl;
}
