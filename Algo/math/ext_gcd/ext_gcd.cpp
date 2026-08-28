// 拡張ユークリッドの互除法 / GCD / LCM
// ax + by = gcd(a,b) を満たす (x,y) を求める
// 計算量: O(log min(a,b))
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 拡張GCD ===
// ax + by = gcd(a,b) の解 (x,y) を求める. 返り値は gcd(a,b)
long long ext_gcd(long long a, long long b, long long &x, long long &y){
    if(b == 0){
        x = 1; y = 0;
        return a;
    }
    long long x1, y1;
    long long g = ext_gcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

// mod m での a の逆元 (gcd(a,m) = 1 のとき)
long long mod_inv(long long a, long long m){
    long long x, y;
    long long g = ext_gcd(a, m, x, y);
    if(g != 1) return -1; // 逆元なし
    return (x % m + m) % m;
}

// === ライブラリ: 中国剰余定理 (CRT) ===
// x ≡ r1 (mod m1), x ≡ r2 (mod m2) を満たす最小非負整数 x
// 返り値: {r, m} (x ≡ r (mod m)), 解なしなら {0, -1}
pair<long long, long long> crt(long long r1, long long m1, long long r2, long long m2){
    long long x, y;
    long long g = ext_gcd(m1, m2, x, y);
    if((r2 - r1) % g != 0) return {0, -1};
    long long m = m1 / g * m2; // lcm(m1, m2)
    long long t = (r2 - r1) / g % (m2 / g);
    long long r = (r1 + m1 * (t % (m2 / g) * (x % (m2 / g)) % (m2 / g)) % m + m) % m;
    return {r, m};
}

// === 例題: ax ≡ b (mod m) を解く ===
int main(){
    long long a, b, m;
    cin >> a >> b >> m;

    // ax ≡ b (mod m) → ax + my = b
    long long x, y;
    long long g = ext_gcd(a, m, x, y);
    if(b % g != 0){
        cout << -1 << endl; // 解なし
    } else {
        x = x * (b / g) % (m / g);
        x = (x + m / g) % (m / g);
        cout << x << endl;
    }
    return 0;
}
