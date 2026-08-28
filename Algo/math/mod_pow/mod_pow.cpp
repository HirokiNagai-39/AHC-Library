// 繰り返し二乗法 (mod_pow) / 累乗の高速計算
// 例題: ABC167 D - Teleporter (ダブリングにも応用可)
// 計算量: O(log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: mod_pow ===
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

// === 例題: ABC167 D - Teleporter ===
// 町iからA[i]にテレポート. K回テレポート後の町を求める
// (ダブリングで解く)
int main(){
    int N;
    long long K;
    cin >> N >> K;
    vector<int> A(N);
    for(int i = 0; i < N; i++){
        cin >> A[i];
        A[i]--;
    }

    // ダブリング: nxt[k][v] = vから2^k回移動した先
    int LOG = 0;
    while((1LL << LOG) <= K) LOG++;
    vector<vector<int>> nxt(LOG, vector<int>(N));
    for(int v = 0; v < N; v++) nxt[0][v] = A[v];
    for(int k = 0; k + 1 < LOG; k++){
        for(int v = 0; v < N; v++){
            nxt[k+1][v] = nxt[k][nxt[k][v]];
        }
    }

    int pos = 0;
    for(int k = 0; k < LOG; k++){
        if((K >> k) & 1) pos = nxt[k][pos];
    }
    cout << pos + 1 << endl;
    return 0;
}
