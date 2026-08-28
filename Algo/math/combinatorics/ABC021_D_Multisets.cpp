// ABC021 D - 多重辺
// https://atcoder.jp/contests/abc021/tasks/abc021_d
// a->bの最短経路数 = 重複組み合わせ H(k, b-a) = C(k+b-a-1, b-a)
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;
static constexpr int MAXN = 200002;
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

long long H(int n, int r) {
    if (n == 0 && r == 0) return 1;
    return C(n + r - 1, r);
}

int main() {
    int n, a, b, k;
    cin >> n >> a >> b >> k;
    init(200001);
    cout << H(k, b - a) << endl;
    return 0;
}
