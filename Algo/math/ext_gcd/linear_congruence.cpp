// 一次合同方程式 ax ≡ b (mod m) の全解
// g = gcd(a,m) が b を割り切らないなら解なし
// 解は x ≡ x0 (mod m/g) の形で g 個
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

// ax ≡ b (mod m) の最小非負解と周期を返す
// 解なし: {-1, -1}
pair<ll, ll> solve_linear_congruence(ll a, ll b, ll m) {
    a = ((a % m) + m) % m;
    b = ((b % m) + m) % m;
    ll x, y;
    ll g = ext_gcd(a, m, x, y);
    if (b % g != 0) return {-1, -1};
    ll period = m / g;
    ll x0 = (x % period * (b / g % period) % period + period) % period;
    return {x0, period};
}

int main() {
    ll a, b, m;
    cin >> a >> b >> m;
    auto [x0, period] = solve_linear_congruence(a, b, m);
    if (x0 == -1) {
        cout << "No solution" << endl;
    } else {
        cout << "x ≡ " << x0 << " (mod " << period << ")" << endl;
        cout << "Solutions in [0, " << m << "): ";
        for (ll x = x0; x < m; x += period) cout << x << " ";
        cout << endl;
    }
    return 0;
}
