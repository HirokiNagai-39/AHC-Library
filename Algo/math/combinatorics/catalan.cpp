// カタラン数 C(n) = C(2n,n)/(n+1)
// 括弧列の数, 二分木の形状数, 対角線を超えない格子路 等
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;
static constexpr int MAXN = 400002;
long long fact[MAXN], inv_fact[MAXN];

long long mod_pow(long long b, long long e, long long m = MOD) {
    long long r = 1; b %= m;
    while (e > 0) { if (e & 1) r = r * b % m; b = b * b % m; e >>= 1; }
    return r;
}

void init(int n) {
    fact[0] = 1;
    for (int i = 1; i <= n; i++) fact[i] = fact[i-1] * i % MOD;
    inv_fact[n] = mod_pow(fact[n], MOD - 2);
    for (int i = n-1; i >= 0; i--) inv_fact[i] = inv_fact[i+1] * (i+1) % MOD;
}

long long C(int n, int r) {
    if (r < 0 || r > n) return 0;
    return fact[n] % MOD * inv_fact[r] % MOD * inv_fact[n-r] % MOD;
}

long long catalan(int n) {
    return C(2*n, n) % MOD * mod_pow(n+1, MOD-2) % MOD;
}

int main() {
    int N;
    cin >> N;
    init(2*N);
    // 出力: C(0)~C(N)
    for (int i = 0; i <= N; i++)
        cout << catalan(i) << "\n";
    return 0;
}
