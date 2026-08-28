// ABC032 D - Knapsack
// https://atcoder.jp/contests/abc032/tasks/abc032_d
// Knapsack where solution depends on parameter ranges:
//   N<=30: meet in the middle (2^(N/2) * log)
//   W<=200000: standard weight-based DP
//   v<=200000: value-based DP
#include <bits/stdc++.h>
using namespace std;

int main(){
    int n; long long W;
    cin >> n >> W;
    vector<long long> w(n), v(n);
    for(int i = 0; i < n; i++) cin >> v[i] >> w[i];

    long long maxv = *max_element(v.begin(), v.end());
    long long maxw = *max_element(w.begin(), w.end());

    if(n <= 30){
        // Meet in the middle
        int h = n / 2;
        auto gen = [&](int l, int r) -> vector<pair<long long,long long>> {
            vector<pair<long long,long long>> res; // (weight, value)
            int sz = r - l;
            for(int mask = 0; mask < (1 << sz); mask++){
                long long sw = 0, sv = 0;
                for(int i = 0; i < sz; i++){
                    if(mask >> i & 1){ sw += w[l+i]; sv += v[l+i]; }
                }
                if(sw <= W) res.push_back({sw, sv});
            }
            sort(res.begin(), res.end());
            // Make value monotonically increasing with weight
            vector<pair<long long,long long>> pruned;
            long long best = -1;
            for(auto &[ww, vv] : res){
                if(vv > best){ pruned.push_back({ww, vv}); best = vv; }
            }
            return pruned;
        };
        auto A = gen(0, h);
        auto B = gen(h, n);
        long long ans = 0;
        for(auto &[wa, va] : A){
            long long rem = W - wa;
            // Binary search in B for largest weight <= rem
            int lo = 0, hi = (int)B.size() - 1, pos = -1;
            while(lo <= hi){
                int mid = (lo + hi) / 2;
                if(B[mid].first <= rem){ pos = mid; lo = mid + 1; }
                else hi = mid - 1;
            }
            if(pos >= 0) ans = max(ans, va + B[pos].second);
        }
        cout << ans << endl;
    } else if(maxw <= 200000 && W <= 200000){
        // Standard 0-1 knapsack on weight
        vector<long long> dp(W + 1, 0);
        for(int i = 0; i < n; i++)
            for(long long j = W; j >= w[i]; j--)
                dp[j] = max(dp[j], dp[j - w[i]] + v[i]);
        cout << *max_element(dp.begin(), dp.end()) << endl;
    } else {
        // Value-based DP: dp[j] = min weight to achieve value j
        long long sumv = 0;
        for(int i = 0; i < n; i++) sumv += v[i];
        vector<long long> dp(sumv + 1, 1e18);
        dp[0] = 0;
        for(int i = 0; i < n; i++)
            for(long long j = sumv; j >= v[i]; j--)
                dp[j] = min(dp[j], dp[j - v[i]] + w[i]);
        long long ans = 0;
        for(long long j = 0; j <= sumv; j++)
            if(dp[j] <= W) ans = j;
        cout << ans << endl;
    }
}
