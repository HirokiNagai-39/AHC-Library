// ABC293 E - Geometric Progression
// https://atcoder.jp/contests/abc293/tasks/abc293_e
// S = 1 + A + A^2 + ... + A^(X-1) mod M
// 繰り返し二乗法で等比級数を計算。f(n) = 1+A+...+A^(n-1)
// f(2k) = f(k) * (1 + A^k), f(2k+1) = f(2k) + A^(2k)
// 計算量: O(log X * log X) or O(log^2 X)
#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll mod_pow(ll base, ll exp, ll mod) {
    ll res = 1; base %= mod;
    while (exp > 0) {
        if (exp & 1) res = res % mod * (base % mod) % mod;
        base = base % mod * (base % mod) % mod;
        exp >>= 1;
    }
    return res;
}

// 1 + A + A^2 + ... + A^(n-1) mod m
ll geo_sum(ll A, ll n, ll m) {
    if (n == 0) return 0;
    if (n == 1) return 1 % m;
    if (n % 2 == 0) {
        // f(2k) = f(k) * (1 + A^k)
        ll half = geo_sum(A, n / 2, m);
        ll pk = mod_pow(A, n / 2, m);
        return half % m * ((1 + pk) % m) % m;
    } else {
        // f(n) = f(n-1) + A^(n-1)
        ll prev = geo_sum(A, n - 1, m);
        return (prev + mod_pow(A, n - 1, m)) % m;
    }
}

int main() {
    ll A, X, M;
    cin >> A >> X >> M;
    cout << geo_sum(A, X, M) << endl;
    return 0;
}
