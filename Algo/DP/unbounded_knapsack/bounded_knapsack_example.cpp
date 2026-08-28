// Bounded Knapsack with Binary Decomposition
// N items, each with value v[i], weight w[i], count c[i].
// Maximize value with total weight <= W.
// Binary decomposition: split c[i] copies into groups of 1,2,4,...,remainder
// and solve as 0-1 knapsack. O(NW * log(max_c))
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,W; cin>>N>>W;
    vector<int> v(N),w(N),c(N);
    for(int i=0;i<N;i++) cin>>v[i]>>w[i]>>c[i];
    // Binary decomposition
    vector<int> bv,bw; // decomposed items
    for(int i=0;i<N;i++){
        int rem=c[i];
        int k=1;
        while(rem>0){
            int take=min(k,rem);
            bv.push_back(v[i]*take);
            bw.push_back(w[i]*take);
            rem-=take;
            k*=2;
        }
    }
    // 0-1 knapsack on decomposed items
    vector<long long> dp(W+1,0);
    for(int i=0;i<(int)bv.size();i++){
        for(int j=W;j>=bw[i];j--){
            dp[j]=max(dp[j],dp[j-bw[i]]+bv[i]);
        }
    }
    cout<<dp[W]<<endl;
}
