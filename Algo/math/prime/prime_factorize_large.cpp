// 大きい数(10^18)の素因数分解 - Pollard's rho + Miller-Rabin
// 計算量: O(N^(1/4)) 期待
#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using ull = unsigned long long;

ll mod_mul(ll a, ll b, ll m) {
    return (__int128)a * b % m;
}

ll mod_pow(ll base, ll exp, ll mod) {
    ll res = 1; base %= mod;
    while (exp > 0) {
        if (exp & 1) res = mod_mul(res, base, mod);
        base = mod_mul(base, base, mod);
        exp >>= 1;
    }
    return res;
}

bool miller_rabin(ll n, ll a) {
    if (n % a == 0) return n == a;
    ll d = n - 1; int r = 0;
    while (d % 2 == 0) { d /= 2; r++; }
    ll x = mod_pow(a, d, n);
    if (x == 1 || x == n - 1) return true;
    for (int i = 0; i < r - 1; i++) {
        x = mod_mul(x, x, n);
        if (x == n - 1) return true;
    }
    return false;
}

bool is_prime(ll n) {
    if (n < 2) return false;
    for (ll a : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37})
        if (!miller_rabin(n, a)) return false;
    return true;
}

ll pollard_rho(ll n) {
    if (n % 2 == 0) return 2;
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    while (true) {
        ll x = rng() % (n - 2) + 2;
        ll c = rng() % (n - 1) + 1;
        ll y = x, d = 1;
        while (d == 1) {
            x = (mod_mul(x, x, n) + c) % n;
            y = (mod_mul(y, y, n) + c) % n;
            y = (mod_mul(y, y, n) + c) % n;
            d = __gcd(abs(x - y), n);
        }
        if (d != n) return d;
    }
}

map<ll, int> factorize(ll n) {
    if (n <= 1) return {};
    if (is_prime(n)) return {{n, 1}};
    ll d = pollard_rho(n);
    auto f1 = factorize(d), f2 = factorize(n / d);
    for (auto& [p, e] : f2) f1[p] += e;
    return f1;
}

int main() {
    ll N;
    cin >> N;
    auto f = factorize(N);
    for (auto& [p, e] : f) cout << p << "^" << e << "\n";
    return 0;
}
