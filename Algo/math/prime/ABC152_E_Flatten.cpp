// ABC152 E - Flatten
// https://atcoder.jp/contests/abc152/tasks/abc152_e
// B[i] = LCM(A)/A[i]。sum(B[i]) mod 10^9+7を求める。
// LCMを素因数分解で管理し、各A[i]の逆元を計算。
// 計算量: O(N sqrt(max A))
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

long long mod_pow(long long b, long long e, long long m = MOD) {
    long long r = 1; b %= m;
    while (e > 0) { if (e & 1) r = r * b % m; b = b * b % m; e >>= 1; }
    return r;
}

int main() {
    int N;
    cin >> N;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    // LCMの素因数分解: 各素因数の最大指数
    map<long long, int> lcm_factors;
    for (auto a : A) {
        long long x = a;
        for (long long p = 2; p * p <= x; p++) {
            int cnt = 0;
            while (x % p == 0) { cnt++; x /= p; }
            lcm_factors[p] = max(lcm_factors[p], cnt);
        }
        if (x > 1) lcm_factors[x] = max(lcm_factors[x], 1);
    }

    // LCM mod MOD
    long long L = 1;
    for (auto& [p, e] : lcm_factors) L = L % MOD * mod_pow(p, e) % MOD;

    long long ans = 0;
    for (auto a : A)
        ans = (ans + L % MOD * mod_pow(a % MOD, MOD - 2) % MOD) % MOD;
    cout << ans << endl;
    return 0;
}
