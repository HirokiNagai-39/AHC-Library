// ABC132 D - Blue and Red Balls
// https://atcoder.jp/contests/abc132/tasks/abc132_d
// K個の青をi箇所の連続区間に分ける方法数 = C(K-1,i-1)*C(N-K+1,i)
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

int main() {
    int N, K;
    cin >> N >> K;
    init(N);
    for (int i = 1; i <= K; i++)
        cout << C(K-1, i-1) * C(N-K+1, i) % MOD << "\n";
    return 0;
}
