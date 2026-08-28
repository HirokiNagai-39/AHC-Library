// TDPC A - Contest
// https://atcoder.jp/contests/tdpc/tasks/tdpc_contest
// N problems with scores p[i]. Find number of distinct achievable total scores.
// Subset sum DP using bitset. O(N * sum/64)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<int> p(N);
    for(auto &x : p) cin >> x;

    int S = 0;
    for(auto x : p) S += x;

    bitset<100001> dp;
    dp[0] = 1;
    for(int i = 0; i < N; i++){
        dp |= (dp << p[i]);
    }
    cout << dp.count() << endl;
}
