// Lucas の定理 (Lucas' Theorem)
// 素数 p が小さいとき、nCr mod p を O(p + log_p(n)) で計算する
// n, r が非常に大きくても p が小さければ高速に求められる
// 計算量: 前処理 O(p), クエリ O(log_p(n))
// 例題: ABC145 D (応用)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Lucas の定理 ===
struct Lucas {
    int p;
    vector<long long> fact, inv_fact;

    static long long mod_pow(long long base, long long exp, long long mod) {
        long long res = 1;
        base %= mod;
        while (exp > 0) {
            if (exp & 1) res = res * base % mod;
            base = base * base % mod;
            exp >>= 1;
        }
        return res;
    }

    Lucas(int p) : p(p), fact(p), inv_fact(p) {
        fact[0] = 1;
        for (int i = 1; i < p; i++) fact[i] = fact[i - 1] * i % p;
        inv_fact[p - 1] = mod_pow(fact[p - 1], p - 2, p);
        for (int i = p - 2; i >= 0; i--) inv_fact[i] = inv_fact[i + 1] * (i + 1) % p;
    }

    // 小さい n, r に対する nCr mod p (n < p)
    long long small_C(int n, int r) {
        if (r < 0 || r > n) return 0;
        return fact[n] * inv_fact[r] % p * inv_fact[n - r] % p;
    }

    // Lucas の定理: nCr mod p (n, r は任意の非負整数)
    long long C(long long n, long long r) {
        if (r < 0 || r > n) return 0;
        long long result = 1;
        while (n > 0 || r > 0) {
            int ni = n % p, ri = r % p;
            result = result * small_C(ni, ri) % p;
            if (result == 0) return 0;
            n /= p;
            r /= p;
        }
        return result;
    }
};

// === 例題: 大きな n, r に対する nCr mod p ===
int main() {
    int p;
    cin >> p;
    Lucas lucas(p);

    int q;
    cin >> q;
    while (q--) {
        long long n, r;
        cin >> n >> r;
        cout << lucas.C(n, r) << "\n";
    }
    return 0;
}
