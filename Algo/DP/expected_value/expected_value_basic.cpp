// 期待値DP 基本パターン
// 例題: ABC280 E - Critical Hit
// https://atcoder.jp/contests/abc280/tasks/abc280_e
// モンスターのHP=N. 攻撃するとP/100の確率で2ダメージ, (100-P)/100で1ダメージ
// 倒すまでの攻撃回数の期待値
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 998244353;

long long mod_pow(long long base, long long exp, long long mod){
    long long result = 1;
    base %= mod;
    while(exp > 0){
        if(exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

long long mod_inv(long long a, long long mod){
    return mod_pow(a, mod - 2, mod);
}

// === 期待値DPの考え方 ===
// E[i] = HP残りiから倒すまでの期待攻撃回数
// E[0] = 0
// E[i] = 1 + (P/100)*E[max(0,i-2)] + ((100-P)/100)*E[i-1]

int main(){
    long long N, P;
    cin >> N >> P;

    long long p2 = P % MOD * mod_inv(100, MOD) % MOD;     // 2ダメージの確率
    long long p1 = (100 - P) % MOD * mod_inv(100, MOD) % MOD; // 1ダメージの確率

    vector<long long> E(N + 1, 0);
    for(int i = 1; i <= N; i++){
        E[i] = 1;
        E[i] = (E[i] + p1 % MOD * E[i-1]) % MOD;
        if(i >= 2) E[i] = (E[i] + p2 % MOD * E[i-2]) % MOD;
        // i=1のとき2ダメージならHP0になるので E[max(0,i-2)] = E[0] = 0 (加算不要…ではない)
        // i=1の場合: E[1] = 1 + p1*E[0] + p2*E[0] = 1 (ではなくE[-1]=E[0]=0)
        // 実は i >= 2 のとき E[i-2], i == 1 のとき E[0] = 0
    }
    // 修正: i=1の場合もp2*E[0] = 0 なので上のコードで正しい

    cout << E[N] << endl;
    return 0;
}
