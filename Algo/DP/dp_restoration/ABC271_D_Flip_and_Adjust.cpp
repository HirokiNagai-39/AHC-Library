// ABC271 D - Flip and Adjust
// https://atcoder.jp/contests/abc271/tasks/abc271_d
// N cards with front a[i] and back b[i]. Choose one side each to sum to S.
// DP + restoration. O(NS)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,S; cin>>N>>S;
    vector<int> a(N),b(N);
    for(int i=0;i<N;i++) cin>>a[i]>>b[i];
    // dp[i][j] = can we make sum j using first i cards?
    // Use 1D with restoration
    vector<vector<int>> dp(N+1,vector<int>(S+1,-1)); // -1=unreachable, 0=front, 1=back
    vector<vector<bool>> reach(N+1,vector<bool>(S+1,false));
    reach[0][0]=true;
    for(int i=0;i<N;i++){
        for(int j=0;j<=S;j++){
            if(!reach[i][j]) continue;
            if(j+a[i]<=S){
                reach[i+1][j+a[i]]=true;
                dp[i+1][j+a[i]]=0; // chose front
            }
            if(j+b[i]<=S){
                reach[i+1][j+b[i]]=true;
                if(!reach[i+1][j+b[i]] || dp[i+1][j+b[i]]==-1)
                    ; // already set above potentially
                // overwrite is fine, we just need any valid path
                if(dp[i+1][j+b[i]]==-1) dp[i+1][j+b[i]]=1;
            }
        }
        // Fix: need proper tracking
    }
    // Redo with proper parent tracking
    // dp2[j] = {reachable, choice at last step, previous sum}
    struct State { bool ok; int choice; int prev; };
    vector<vector<State>> st(N+1,vector<State>(S+1,{false,-1,-1}));
    st[0][0].ok=true;
    for(int i=0;i<N;i++){
        for(int j=0;j<=S;j++){
            if(!st[i][j].ok) continue;
            if(j+a[i]<=S && !st[i+1][j+a[i]].ok){
                st[i+1][j+a[i]]={true,0,j};
            }
            if(j+b[i]<=S && !st[i+1][j+b[i]].ok){
                st[i+1][j+b[i]]={true,1,j};
            }
        }
    }
    if(!st[N][S].ok){ cout<<"No"<<endl; return 0; }
    cout<<"Yes"<<endl;
    string res;
    int cur=S;
    for(int i=N;i>=1;i--){
        res+=(st[i][cur].choice==0?'H':'T');
        cur=st[i][cur].prev;
    }
    reverse(res.begin(),res.end());
    cout<<res<<endl;
}
