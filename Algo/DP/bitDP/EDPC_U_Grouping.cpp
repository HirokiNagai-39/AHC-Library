// EDPC U - Grouping
// https://atcoder.jp/contests/dp/tasks/dp_u
// Partition N people into groups to maximize total score.
// Precompute group scores, then subset DP. O(3^N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<vector<long long>> a(N,vector<long long>(N));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) cin>>a[i][j];
    int full=1<<N;
    // precompute score for each subset
    vector<long long> score(full,0);
    for(int S=0;S<full;S++){
        vector<int> vs;
        for(int i=0;i<N;i++) if(S>>i&1) vs.push_back(i);
        for(int i=0;i<(int)vs.size();i++)
            for(int j=i+1;j<(int)vs.size();j++)
                score[S]+=a[vs[i]][vs[j]];
    }
    vector<long long> dp(full, -1e18);
    dp[0]=0;
    for(int S=1;S<full;S++){
        for(int T=S;T>0;T=(T-1)&S){
            dp[S]=max(dp[S],dp[S^T]+score[T]);
        }
    }
    cout<<dp[full-1]<<endl;
}
