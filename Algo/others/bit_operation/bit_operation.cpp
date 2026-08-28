// ビット演算テクニック集
// 競プロでよく使うビット操作
#include <bits/stdc++.h>
using namespace std;

int main(){
    // ========== 基本操作 ==========
    int x = 13; // 二進: 1101

    // i番目のビットを取得 (0-indexed)
    int bit_i = (x >> 2) & 1; // 1

    // i番目のビットを立てる
    x |= (1 << 2);

    // i番目のビットを消す
    x &= ~(1 << 2);

    // i番目のビットを反転
    x ^= (1 << 2);

    // ========== 部分集合列挙 ==========
    // 集合Sの部分集合を全列挙 (空集合含む)
    int S = 0b1011; // 例: {0, 1, 3}
    for(int sub = S; ; sub = (sub - 1) & S){
        // sub は S の部分集合
        cout << sub << " ";
        if(sub == 0) break;
    }
    cout << endl;

    // ========== ビット全探索 ==========
    int N = 5;
    for(int bit = 0; bit < (1 << N); bit++){
        // bit は N 要素の部分集合
        for(int i = 0; i < N; i++){
            if(bit >> i & 1){
                // 要素 i が選ばれている
            }
        }
    }

    // ========== 便利関数 ==========
    // ビットの1の個数
    int cnt = __builtin_popcount(x);
    long long y = 123456789LL;
    int cnt2 = __builtin_popcountll(y);

    // 最下位ビット (LSB)
    int lsb = x & (-x);

    // 最下位ビットの位置
    int lsb_pos = __builtin_ctz(x); // x != 0 のとき

    // 最上位ビットの位置 (0-indexed)
    int msb_pos = 31 - __builtin_clz(x); // x != 0 のとき
    // long long版: 63 - __builtin_clzll(y)

    // N以上の最小の2冪
    int pw2 = 1;
    while(pw2 < N) pw2 <<= 1;

    (void)bit_i; (void)cnt; (void)cnt2;
    (void)lsb; (void)lsb_pos; (void)msb_pos; (void)pw2;

    return 0;
}
