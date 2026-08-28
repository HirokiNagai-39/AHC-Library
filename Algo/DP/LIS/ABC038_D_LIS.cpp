// ABC038 D - LIS
// https://atcoder.jp/contests/abc038/tasks/abc038_d
// Find the length of the Longest Increasing Subsequence. O(NlogN)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<int> A(N);
    for(auto &x : A) cin >> x;

    // dp[i] = smallest tail element of increasing subsequence of length i+1
    vector<int> dp;
    for(int i = 0; i < N; i++){
        auto it = lower_bound(dp.begin(), dp.end(), A[i]);
        if(it == dp.end()) dp.push_back(A[i]);
        else *it = A[i];
    }
    cout << dp.size() << endl;
}
