// ABC034 C - 経路
// https://atcoder.jp/contests/abc034/tasks/abc034_c
// (1,1)から(W,H)への経路数 = C(W+H-2, H-1) mod 10^9+7
// 分数のmod計算にフェルマーの小定理で逆元を使用
// 計算量: O(N)
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

int main() {
    long long W, H;
    cin >> W >> H;
    long long n = W + H - 2, r = H - 1;
    long long num = 1, den = 1;
    for (long long i = 0; i < r; i++) {
        num = num * ((n - i) % MOD) % MOD;
        den = den * ((i + 1) % MOD) % MOD;
    }
    cout << num % MOD * mod_pow(den, MOD - 2) % MOD << endl;
    return 0;
}
