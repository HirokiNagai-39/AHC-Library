// NTT (Number Theoretic Transform, 数論変換)
// 多項式の畳み込み (mod 998244353) を O(N log N) で行う
// 計算量: O(N log N)
// 例題: ATC001 C, ABC196 F
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: NTT (mod 998244353) ===
// 998244353 = 119 * 2^23 + 1, 原始根 g = 3
struct NTT {
    static constexpr long long MOD = 998244353;
    static constexpr long long g = 3;

    static long long mod_pow(long long base, long long exp, long long mod = MOD) {
        long long res = 1;
        base %= mod;
        while (exp > 0) {
            if (exp & 1) res = res * base % mod;
            base = base * base % mod;
            exp >>= 1;
        }
        return res;
    }

    static void ntt(vector<long long>& a, bool inverse) {
        int n = a.size();
        for (int i = 1, j = 0; i < n; i++) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) swap(a[i], a[j]);
        }
        for (int len = 2; len <= n; len <<= 1) {
            long long w = inverse ? mod_pow(g, MOD - 1 - (MOD - 1) / len)
                                  : mod_pow(g, (MOD - 1) / len);
            for (int i = 0; i < n; i += len) {
                long long wn = 1;
                for (int j = 0; j < len / 2; j++) {
                    long long u = a[i + j];
                    long long v = a[i + j + len / 2] * wn % MOD;
                    a[i + j] = (u + v) % MOD;
                    a[i + j + len / 2] = (u - v + MOD) % MOD;
                    wn = wn * w % MOD;
                }
            }
        }
        if (inverse) {
            long long n_inv = mod_pow(n, MOD - 2);
            for (auto& x : a) x = x * n_inv % MOD;
        }
    }

    // 畳み込み: c[k] = Σ a[i]*b[k-i]
    static vector<long long> convolution(vector<long long> a, vector<long long> b) {
        int result_size = a.size() + b.size() - 1;
        int n = 1;
        while (n < result_size) n <<= 1;
        a.resize(n); b.resize(n);
        ntt(a, false); ntt(b, false);
        for (int i = 0; i < n; i++) a[i] = a[i] * b[i] % MOD;
        ntt(a, true);
        a.resize(result_size);
        return a;
    }
};

// === 例題: 多項式の積 ===
// 入力: 次数 N, M と係数列 a[0..N], b[0..M]
// 出力: a*b の係数列
int main() {
    int n, m;
    cin >> n >> m;
    vector<long long> a(n + 1), b(m + 1);
    for (int i = 0; i <= n; i++) cin >> a[i];
    for (int i = 0; i <= m; i++) cin >> b[i];

    auto c = NTT::convolution(a, b);

    for (int i = 0; i < (int)c.size(); i++) {
        cout << c[i] << " \n"[i == (int)c.size() - 1];
    }
    return 0;
}
