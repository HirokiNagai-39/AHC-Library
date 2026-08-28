// ABC267 D - Index x A(Not Continuous ver.)
// https://atcoder.jp/contests/abc267/tasks/abc267_d
// Choose M elements from length-N sequence, maximize sum of i*A[p_i].
// dp[j] = max weighted sum when j elements chosen so far. O(NM)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N, M;
    cin >> N >> M;
    vector<long long> A(N);
    for(auto &x : A) cin >> x;

    // dp[j] = max sum when we have chosen j elements so far
    // When we pick A[i] as the j-th chosen element, contribution is j * A[i]
    static constexpr long long NEG = -1e18;
    vector<long long> dp(M + 1, NEG);
    dp[0] = 0;

    for(int i = 0; i < N; i++){
        // Iterate j in reverse to avoid using A[i] multiple times
        for(int j = min(i + 1, M); j >= 1; j--){
            if(dp[j-1] == NEG) continue;
            dp[j] = max(dp[j], dp[j-1] + (long long)j * A[i]);
        }
    }
    cout << dp[M] << endl;
}
