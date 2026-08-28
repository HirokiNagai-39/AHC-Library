// Lagrange 補間 (Lagrange Interpolation)
// n+1 個の点 (x_0, y_0), ..., (x_n, y_n) を通る n 次多項式の値を求める
// 計算量: 一般版 O(N^2), 連続点版 O(N)
// 例題: ABC208 F (応用), 典型90 #015
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 998244353;

long long mod_pow(long long base, long long exp, long long mod = MOD) {
    long long res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) res = res * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return res;
}

long long mod_inv(long long a, long long mod = MOD) {
    return mod_pow(a, mod - 2, mod);
}

// === ライブラリ: Lagrange 補間 (一般版) ===
// 点 (xs[i], ys[i]) を通る多項式を t で評価
// O(N^2)
long long lagrange(const vector<long long>& xs, const vector<long long>& ys, long long t) {
    int n = xs.size();
    long long ans = 0;
    for (int i = 0; i < n; i++) {
        long long num = ys[i], den = 1;
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            num = num % MOD * ((t - xs[j]) % MOD + MOD) % MOD;
            den = den % MOD * ((xs[i] - xs[j]) % MOD + MOD) % MOD;
        }
        ans = (ans + num % MOD * mod_inv(den)) % MOD;
    }
    return ans;
}

// === ライブラリ: Lagrange 補間 (連続点版, 推奨) ===
// x_i = 0, 1, 2, ..., n のときの高速版
// y[i] = f(i) として f(t) を求める
// O(N)
long long lagrange_fast(const vector<long long>& y, long long t) {
    int n = y.size(); // n+1 個の点 (0..n)
    if (t < n) return y[t];

    // 前処理: 階乗の逆元
    vector<long long> fact(n), inv_fact(n);
    fact[0] = 1;
    for (int i = 1; i < n; i++) fact[i] = fact[i - 1] * i % MOD;
    inv_fact[n - 1] = mod_inv(fact[n - 1]);
    for (int i = n - 2; i >= 0; i--) inv_fact[i] = inv_fact[i + 1] * (i + 1) % MOD;

    // prefix[i] = Π_{j=0}^{i-1} (t - j)
    // suffix[i] = Π_{j=i+1}^{n-1} (t - j)
    vector<long long> prefix(n + 1, 1), suffix(n + 1, 1);
    for (int i = 0; i < n; i++) prefix[i + 1] = prefix[i] % MOD * ((t - i) % MOD + MOD) % MOD;
    for (int i = n - 1; i >= 0; i--) suffix[i] = suffix[i + 1] % MOD * ((t - i) % MOD + MOD) % MOD;

    long long ans = 0;
    for (int i = 0; i < n; i++) {
        long long num = y[i] % MOD * prefix[i] % MOD * suffix[i + 1] % MOD;
        long long den = inv_fact[i] % MOD * inv_fact[n - 1 - i] % MOD;
        if ((n - 1 - i) & 1) den = (MOD - den) % MOD;
        ans = (ans + num % MOD * den) % MOD;
    }
    return ans;
}

// === 例題: 1^k + 2^k + ... + n^k を求める ===
// f(n) = Σ_{i=1}^{n} i^k は k+1 次多項式
// f(0), f(1), ..., f(k+1) を計算して Lagrange 補間
int main() {
    long long n, k;
    cin >> n >> k;
    // f(i) = 1^k + 2^k + ... + i^k を k+2 個求める
    int sz = k + 2;
    vector<long long> y(sz);
    y[0] = 0;
    for (int i = 1; i < sz; i++) {
        y[i] = (y[i - 1] + mod_pow(i, k)) % MOD;
    }
    cout << lagrange_fast(y, n) << "\n";
    return 0;
}
