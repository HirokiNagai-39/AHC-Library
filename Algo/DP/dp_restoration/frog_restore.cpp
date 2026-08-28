// EDPC A - Frog (with Path Restoration)
// https://atcoder.jp/contests/dp/tasks/dp_a
// Frog jumps 1 or 2 stones. Min cost to reach stone N. Restore the path taken.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<int> h(N);
    for(int i=0;i<N;i++) cin>>h[i];
    vector<int> dp(N,1e9);
    vector<int> from(N,-1);
    dp[0]=0;
    for(int i=1;i<N;i++){
        if(dp[i-1]+abs(h[i]-h[i-1])<dp[i]){
            dp[i]=dp[i-1]+abs(h[i]-h[i-1]);
            from[i]=i-1;
        }
        if(i>=2 && dp[i-2]+abs(h[i]-h[i-2])<dp[i]){
            dp[i]=dp[i-2]+abs(h[i]-h[i-2]);
            from[i]=i-2;
        }
    }
    cout<<dp[N-1]<<endl;
    // Restore path
    vector<int> path;
    for(int v=N-1;v!=-1;v=from[v]) path.push_back(v);
    reverse(path.begin(),path.end());
    for(int i=0;i<(int)path.size();i++)
        cout<<path[i]+1<<" \n"[i==(int)path.size()-1];
}
