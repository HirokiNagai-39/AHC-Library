// 中国剰余定理 (CRT) 実践例
// 複数の x ≡ r_i (mod m_i) から x を復元
// 計算量: O(N log M)
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

// x ≡ r1 (mod m1) かつ x ≡ r2 (mod m2)
// 返り値: {r, m} (x ≡ r (mod m)), 解なし {0, -1}
pair<ll, ll> crt2(ll r1, ll m1, ll r2, ll m2) {
    ll x, y;
    ll g = ext_gcd(m1, m2, x, y);
    if ((r2 - r1) % g != 0) return {0, -1};
    ll m = m1 / g * m2;
    ll t = (r2 - r1) / g % (m2 / g);
    ll r = ((r1 + m1 % m * (t % (m2/g) % m) % m * (x % (m2/g) % m)) % m + m) % m;
    return {r, m};
}

// 複数条件を順次マージ
pair<ll, ll> crt(const vector<ll>& r, const vector<ll>& m) {
    ll R = 0, M = 1;
    for (int i = 0; i < (int)r.size(); i++) {
        auto [nr, nm] = crt2(R, M, r[i], m[i]);
        if (nm == -1) return {0, -1};
        R = nr; M = nm;
    }
    return {R, M};
}

int main() {
    int N;
    cin >> N;
    vector<ll> r(N), m(N);
    for (int i = 0; i < N; i++) cin >> r[i] >> m[i];
    auto [R, M] = crt(r, m);
    if (M == -1) cout << -1 << endl;
    else cout << R << " (mod " << M << ")" << endl;
    return 0;
}
