// Wythoff's Game
// Two piles of stones. Each turn, take any number from one pile,
// or equal number from both piles. Last to move wins.
// Cold positions (P-positions): (floor(k*phi), floor(k*phi^2)) for k=0,1,2,...
// where phi = (1+sqrt(5))/2.
// Given (a,b), determine winner.
#include <bits/stdc++.h>
using namespace std;
int main(){
    int T; cin>>T;
    while(T--){
        long long a,b; cin>>a>>b;
        if(a>b) swap(a,b);
        double phi=(1.0+sqrt(5.0))/2.0;
        long long k=(long long)((b-a)*phi);
        // Check if (a,b) is a P-position (cold position)
        // P-position iff a = floor(k*phi) and b = floor(k*phi^2) where k = b-a
        long long diff=b-a;
        long long expected_a=(long long)(diff*phi);
        if(expected_a==a)
            cout<<"Second"<<endl; // P-position, second player wins
        else
            cout<<"First"<<endl;  // N-position, first player wins
    }
}
// Alternative: DP approach for small values
// dp[a][b] = true if (a,b) is N-position (winning for current player)
// dp[0][0] = false
// dp[a][b] = true if any reachable state is losing for opponent
