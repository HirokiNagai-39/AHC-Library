// 二項係数 nCr mod p (前計算テーブル)
// 例題: ABC021 D - 多重辺
// https://atcoder.jp/contests/abc021/tasks/abc021_d
// 計算量: 前処理 O(N), クエリ O(1)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

// === ライブラリ: Combination (nCr mod p) ===
struct Combination {
    int n;
    vector<long long> fact, inv_fact;

    Combination(int n) : n(n), fact(n + 1), inv_fact(n + 1) {
        fact[0] = 1;
        for(int i = 1; i <= n; i++) fact[i] = fact[i-1] * i % MOD;
        inv_fact[n] = mod_pow(fact[n], MOD - 2);
        for(int i = n - 1; i >= 0; i--) inv_fact[i] = inv_fact[i+1] * (i+1) % MOD;
    }

    static long long mod_pow(long long base, long long exp, long long mod = MOD){
        long long result = 1;
        base %= mod;
        while(exp > 0){
            if(exp & 1) result = result * base % mod;
            base = base * base % mod;
            exp >>= 1;
        }
        return result;
    }

    // nCr
    long long C(int n, int r){
        if(r < 0 || r > n) return 0;
        return fact[n] % MOD * inv_fact[r] % MOD * inv_fact[n-r] % MOD;
    }

    // nPr
    long long P(int n, int r){
        if(r < 0 || r > n) return 0;
        return fact[n] % MOD * inv_fact[n-r] % MOD;
    }

    // nHr (重複組み合わせ) = C(n+r-1, r)
    long long H(int n, int r){
        if(n == 0 && r == 0) return 1;
        return C(n + r - 1, r);
    }
};

// === 例題: ABC021 D ===
// a→bの最短経路数 = C(a+b, a) の形 (重複組み合わせ)
int main(){
    int a, b, k;
    cin >> a >> b >> k;
    // k個から重複ありでb-a個選ぶ = H(k, b-a) = C(k+b-a-1, b-a)
    Combination comb(200001);
    cout << comb.H(k, b - a) << endl;
    return 0;
}
