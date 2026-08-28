// ABC228 E - Integer Sequence Fair
// https://atcoder.jp/contests/abc228/tasks/abc228_e
// M^(K^N) mod P。フェルマーの小定理: K^N mod (P-1) を先に計算。
// P=998244353 (素数)
// 計算量: O(log N + log K)
#include <bits/stdc++.h>
using namespace std;

using ll = long long;
static constexpr ll MOD = 998244353;

ll mod_pow(ll base, ll exp, ll mod) {
    ll res = 1; base %= mod;
    if (base < 0) base += mod;
    while (exp > 0) {
        if (exp & 1) res = res * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return res;
}

int main() {
    ll N, K, M;
    cin >> N >> K >> M;
    if (M % MOD == 0) {
        cout << 0 << endl;
        return 0;
    }
    // M^(K^N) mod P
    // K^N mod (P-1) by Fermat's little theorem
    ll e = mod_pow(K, N, MOD - 1);
    cout << mod_pow(M, e, MOD) << endl;
    return 0;
}
