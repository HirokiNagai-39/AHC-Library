// mod逆元の計算例
// gcd(a,m)=1 のとき拡張GCDでa^{-1} mod mを求める
// gcd(a,m)!=1 の場合は逆元が存在しないことを検出
// 計算量: O(log m)
#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll ext_gcd(ll a, ll b, ll &x, ll &y) {
    if (b == 0) { x = 1; y = 0; return a; }
    ll x1, y1;
    ll g = ext_gcd(b, a % b, x1, y1);
    x = y1; y = x1 - (a / b) * y1;
    return g;
}

// a^{-1} mod m (存在しなければ-1)
ll mod_inv(ll a, ll m) {
    a = ((a % m) + m) % m;
    ll x, y;
    ll g = ext_gcd(a, m, x, y);
    if (g != 1) return -1;
    return (x % m + m) % m;
}

// フェルマーの小定理版 (mが素数限定)
ll mod_inv_fermat(ll a, ll m) {
    ll res = 1, b = m - 2;
    a %= m;
    while (b > 0) {
        if (b & 1) res = res * a % m;
        a = a * a % m;
        b >>= 1;
    }
    return res;
}

int main() {
    ll a, m;
    cin >> a >> m;
    ll inv = mod_inv(a, m);
    if (inv == -1)
        cout << "No inverse (gcd=" << __gcd(a, m) << ")" << endl;
    else {
        cout << inv << endl;
        // 検算: a * inv mod m == 1
        cout << "Check: " << a << " * " << inv << " mod " << m << " = "
             << (a % m * inv % m) % m << endl;
    }
    return 0;
}
