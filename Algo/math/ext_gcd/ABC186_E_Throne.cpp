// ABC186 E - Throne
// https://atcoder.jp/contests/abc186/tasks/abc186_e
// S + K*x ≡ 0 (mod N) を解く。x = -S/K mod N。拡張GCDで逆元計算。
// gcd(K,N)がSを割り切らないなら解なし。
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll ext_gcd(ll a, ll b, ll &x, ll &y) {
    if (b == 0) { x = 1; y = 0; return a; }
    ll x1, y1;
    ll g = ext_gcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

int main() {
    int T;
    cin >> T;
    while (T--) {
        ll N, S, K;
        cin >> N >> S >> K;
        ll x, y;
        ll g = ext_gcd(K, N, x, y);
        if (S % g != 0) {
            cout << -1 << "\n";
        } else {
            ll mod = N / g;
            // K/g * x ≡ (N-S)/g (mod N/g)
            ll ans = (-(S / g) % mod * (x % mod) % mod + mod) % mod;
            cout << ans << "\n";
        }
    }
    return 0;
}
