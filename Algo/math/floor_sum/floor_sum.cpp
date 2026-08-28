// Floor Sum (床関数の和)
// Σ_{i=0}^{n-1} floor((a*i + b) / m) を O(log m) で計算する
// Hermite の互除法に基づくアルゴリズム (互除法と同様に軸を入れ替えて再帰)
// 計算量: O(log(a + m))
// 例題: ABC283 Ex, Practice2 C (ACL)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Floor Sum ===
// Σ_{i=0}^{n-1} floor((a*i + b) / m)
// 条件: n >= 0, m >= 1, a >= 0, b >= 0
long long floor_sum(long long n, long long m, long long a, long long b) {
    long long ans = 0;
    while (true) {
        if (a >= m) {
            ans += n * (n - 1) / 2 * (a / m);
            a %= m;
        }
        if (b >= m) {
            ans += n * (b / m);
            b %= m;
        }
        long long y_max = a * n + b;
        if (y_max < m) break;
        n = y_max / m;
        b = y_max % m;
        swap(m, a);
    }
    return ans;
}

// === 例題: Practice2 C - Floor Sum ===
int main() {
    int T;
    cin >> T;
    while (T--) {
        long long n, m, a, b;
        cin >> n >> m >> a >> b;
        cout << floor_sum(n, m, a, b) << "\n";
    }
    return 0;
}
