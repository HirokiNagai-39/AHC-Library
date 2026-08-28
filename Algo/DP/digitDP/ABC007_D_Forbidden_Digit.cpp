// ABC007 D - Forbidden Digit
// https://atcoder.jp/contests/abc007/tasks/abc007_d
// Count numbers in [A,B] that do NOT contain digits 4 or 9. Digit DP.
#include <bits/stdc++.h>
using namespace std;

long long solve(string N){
    int n=N.size();
    // dp[pos][tight]
    vector<vector<long long>> memo(n+1,vector<long long>(2,-1));
    function<long long(int,bool)> dp=[&](int pos,bool tight)->long long{
        if(pos==n) return 1; // count the number itself
        if(memo[pos][tight]!=-1) return memo[pos][tight];
        int lim=tight?(N[pos]-'0'):9;
        long long res=0;
        for(int d=0;d<=lim;d++){
            if(d==4||d==9) continue;
            res+=dp(pos+1,tight&&(d==lim));
        }
        return memo[pos][tight]=res;
    };
    return dp(0,true)-1; // subtract 0
}

int main(){
    long long A,B; cin>>A>>B;
    auto to_str=[](long long x){return to_string(x);};
    cout<<solve(to_str(B))-solve(to_str(A-1))<<endl;
}
