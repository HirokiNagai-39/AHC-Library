// Baby-step Giant-step (離散対数)
// a^x ≡ b (mod m) を満たす最小の非負整数 x を求める
// 計算量: O(√m)
// 例題: ABC270 G (応用)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Baby-step Giant-step ===
// a^x ≡ b (mod m) の最小非負整数解を返す (存在しなければ -1)
// a と m は互いに素でなくてもよい
long long baby_giant_step(long long a, long long b, long long m) {
    a %= m; b %= m;
    if (b == 1 % m) return 0;

    // a と m が互いに素でない場合の前処理
    // a^k / Π g_i ≡ b / Π g_i (mod m / Π g_i) に帰着
    long long k = 0, t = 1;
    while (true) {
        long long g = __gcd(a, m);
        if (g == 1) break;
        if (b % g != 0) return -1;
        b /= g; m /= g;
        t = t * (a / g) % m;
        k++;
        if (t == b) return k;
    }

    // ここで gcd(a, m) = 1
    // t * a^x ≡ b (mod m) を解く
    // Baby-step: b * a^j をテーブルに格納 (j = 0, ..., sq-1)
    // Giant-step: t * (a^sq)^i と一致する j を探す
    // → t * a^(i*sq) = b * a^j → a^(k + i*sq - j) ≡ b (mod 元のm)
    long long sq = (long long)ceil(sqrt((double)m));
    unordered_map<long long, long long> baby;
    long long cur = b;
    for (long long j = 0; j < sq; j++) {
        baby[cur] = j;
        cur = cur * a % m;
    }

    long long giant = t;
    long long a_sq = 1;
    for (long long j = 0; j < sq; j++) a_sq = a_sq * a % m;

    for (long long i = 0; i <= sq; i++) {
        if (baby.count(giant)) {
            return k + i * sq - baby[giant];
        }
        giant = giant * a_sq % m;
    }
    return -1;
}

// === 例題: a^x ≡ b (mod m) の最小解 ===
int main() {
    long long a, b, m;
    cin >> a >> b >> m;
    long long ans = baby_giant_step(a, b, m);
    if (ans == -1) {
        cout << -1 << "\n";
    } else {
        cout << ans << "\n";
    }
    return 0;
}
