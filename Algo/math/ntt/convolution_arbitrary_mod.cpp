// 任意 mod 畳み込み (Arbitrary Mod Convolution)
// NTT-friendly でない mod でも畳み込みを行う
// 3つの NTT-friendly mod で計算し、CRT で復元する
// 計算量: O(N log N)
// 例題: ABC213 F (応用)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 任意 mod 畳み込み ===

// NTT 本体 (テンプレート版)
template<long long MOD, long long G>
struct NTTBase {
    static long long mod_pow(long long base, long long exp) {
        long long res = 1;
        base %= MOD;
        while (exp > 0) {
            if (exp & 1) res = res * base % MOD;
            base = base * base % MOD;
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
            long long w = inverse ? mod_pow(G, MOD - 1 - (MOD - 1) / len)
                                  : mod_pow(G, (MOD - 1) / len);
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

    static vector<long long> convolution(vector<long long> a, vector<long long> b) {
        int result_size = a.size() + b.size() - 1;
        int n = 1;
        while (n < result_size) n <<= 1;
        a.resize(n); b.resize(n);
        for (auto& x : a) x = ((x % MOD) + MOD) % MOD;
        for (auto& x : b) x = ((x % MOD) + MOD) % MOD;
        ntt(a, false); ntt(b, false);
        for (int i = 0; i < n; i++) a[i] = a[i] * b[i] % MOD;
        ntt(a, true);
        a.resize(result_size);
        return a;
    }
};

// 3つの NTT-friendly な素数
using NTT1 = NTTBase<998244353, 3>;    // 119 * 2^23 + 1
using NTT2 = NTTBase<167772161, 3>;    // 5 * 2^25 + 1
using NTT3 = NTTBase<469762049, 3>;    // 7 * 2^26 + 1

// 任意 mod での畳み込み
vector<long long> convolution_mod(vector<long long> a, vector<long long> b, long long mod) {
    auto c1 = NTT1::convolution(a, b);
    auto c2 = NTT2::convolution(a, b);
    auto c3 = NTT3::convolution(a, b);

    static constexpr long long M1 = 998244353, M2 = 167772161, M3 = 469762049;
    // CRT で復元
    // M1_inv_M2 = M1^{-1} mod M2
    long long M1_inv_M2 = NTT2::mod_pow(M1, M2 - 2);
    // M12_inv_M3 = (M1*M2)^{-1} mod M3
    long long M12_inv_M3 = NTT3::mod_pow(M1 % M3 * (M2 % M3) % M3, M3 - 2);

    int n = c1.size();
    vector<long long> result(n);
    for (int i = 0; i < n; i++) {
        // Garner's algorithm
        long long v1 = c1[i];
        long long v2 = (c2[i] - v1 % M2 + M2) % M2 * M1_inv_M2 % M2;
        long long v3 = ((c3[i] - v1 % M3 + M3) % M3 - v2 % M3 * (M1 % M3) % M3 + M3) % M3
                       * M12_inv_M3 % M3;
        // x = v1 + v2*M1 + v3*M1*M2
        result[i] = (v1 % mod + v2 % mod * (M1 % mod) % mod
                    + v3 % mod * (M1 % mod) % mod * (M2 % mod) % mod) % mod;
    }
    return result;
}

// === 例題: mod 1e9+7 での多項式積 ===
int main() {
    static constexpr long long MOD = 1e9 + 7;
    int n, m;
    cin >> n >> m;
    vector<long long> a(n + 1), b(m + 1);
    for (int i = 0; i <= n; i++) cin >> a[i];
    for (int i = 0; i <= m; i++) cin >> b[i];

    auto c = convolution_mod(a, b, MOD);
    for (int i = 0; i < (int)c.size(); i++) {
        cout << c[i] << " \n"[i == (int)c.size() - 1];
    }
    return 0;
}
