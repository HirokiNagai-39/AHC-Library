// ABC156 D - Bouquet
// https://atcoder.jp/contests/abc156/tasks/abc156_d
// N本の花からa,b本以外を選ぶ方法数 = 2^N - 1 - C(N,a) - C(N,b)
// 計算量: O(max(a,b) + log MOD)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

long long mod_pow(long long base, long long exp, long long mod = MOD) {
    long long res = 1; base %= mod;
    while (exp > 0) {
        if (exp & 1) res = res * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return res;
}

// C(n, r) for large n, small r
long long nCr(long long n, long long r) {
    if (r < 0 || r > n) return 0;
    long long num = 1, den = 1;
    for (long long i = 0; i < r; i++) {
        num = num % MOD * ((n - i) % MOD) % MOD;
        den = den % MOD * ((i + 1) % MOD) % MOD;
    }
    return num % MOD * mod_pow(den, MOD - 2) % MOD;
}

int main() {
    long long N, a, b;
    cin >> N >> a >> b;
    long long ans = (mod_pow(2, N) - 1 - nCr(N, a) - nCr(N, b) % MOD + MOD * 3) % MOD;
    cout << ans << endl;
    return 0;
}
