// ModInt (自動mod演算)
// 四則演算を自動でmod計算. +, -, *, / すべて対応
// 計算量: +,-,* O(1), / O(log MOD)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: ModInt ===
template<long long MOD>
struct ModInt {
    long long val;
    ModInt(long long v = 0) : val((v % MOD + MOD) % MOD) {}
    ModInt operator+(const ModInt& o) const { return ModInt(val + o.val); }
    ModInt operator-(const ModInt& o) const { return ModInt(val - o.val + MOD); }
    ModInt operator*(const ModInt& o) const { return ModInt(val * o.val); }
    ModInt operator/(const ModInt& o) const { return *this * o.inv(); }
    ModInt& operator+=(const ModInt& o){ return *this = *this + o; }
    ModInt& operator-=(const ModInt& o){ return *this = *this - o; }
    ModInt& operator*=(const ModInt& o){ return *this = *this * o; }
    ModInt& operator/=(const ModInt& o){ return *this = *this / o; }
    bool operator==(const ModInt& o) const { return val == o.val; }
    bool operator!=(const ModInt& o) const { return val != o.val; }

    ModInt pow(long long n) const {
        ModInt res = 1, base = *this;
        while(n > 0){
            if(n & 1) res *= base;
            base *= base;
            n >>= 1;
        }
        return res;
    }

    ModInt inv() const { return pow(MOD - 2); } // MODが素数のとき

    friend ostream& operator<<(ostream& os, const ModInt& m){ return os << m.val; }
    friend istream& operator>>(istream& is, ModInt& m){
        long long v; is >> v; m = ModInt(v); return is;
    }
};

using mint = ModInt<998244353>;
// using mint = ModInt<1000000007>;

// === 例題: フィボナッチ数 mod p ===
int main(){
    int N;
    cin >> N;
    if(N <= 1){ cout << 1 << endl; return 0; }
    mint a = 1, b = 1;
    for(int i = 2; i <= N; i++){
        mint c = a + b;
        a = b;
        b = c;
    }
    cout << b << endl;
    return 0;
}
