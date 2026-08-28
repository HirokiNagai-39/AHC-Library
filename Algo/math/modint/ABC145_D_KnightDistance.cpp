// ABC145 D - Knight
// https://atcoder.jp/contests/abc145/tasks/abc145_d
// ナイト(1,2)(2,1)移動でX,Yに到達する方法数
// a回(1,2) + b回(2,1) => a+2b=X, 2a+b=Y => a=(2Y-X)/3, b=(2X-Y)/3
// 計算量: O(N) 前処理
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;
static constexpr int MAXN = 1000001;

long long fact[MAXN], inv_fact[MAXN];

long long mod_pow(long long base, long long exp, long long mod = MOD) {
    long long res = 1; base %= mod;
    while (exp > 0) {
        if (exp & 1) res = res * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return res;
}

void init() {
    fact[0] = 1;
    for (int i = 1; i < MAXN; i++) fact[i] = fact[i-1] * i % MOD;
    inv_fact[MAXN-1] = mod_pow(fact[MAXN-1], MOD - 2);
    for (int i = MAXN-2; i >= 0; i--) inv_fact[i] = inv_fact[i+1] * (i+1) % MOD;
}

long long C(int n, int r) {
    if (r < 0 || r > n) return 0;
    return fact[n] % MOD * inv_fact[r] % MOD * inv_fact[n-r] % MOD;
}

int main() {
    int X, Y;
    cin >> X >> Y;
    init();
    if ((X + Y) % 3 != 0) { cout << 0 << endl; return 0; }
    long long b = (2LL * X - Y) / 3;
    long long a = (2LL * Y - X) / 3;
    if (a < 0 || b < 0) { cout << 0 << endl; return 0; }
    cout << C(a + b, a) << endl;
    return 0;
}
