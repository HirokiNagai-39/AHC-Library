// GCD畳み込み (包除原理)
// f[g] = sum of a[i] where g | i を高速に計算し、
// GCD(i,j)=k に関する和を求める
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

using ll = long long;

int main() {
    int N;
    cin >> N;
    vector<ll> a(N + 1);
    for (int i = 1; i <= N; i++) cin >> a[i];

    // cnt[d] = d の倍数にある a[i] の個数
    vector<ll> cnt(N + 1, 0);
    for (int d = 1; d <= N; d++)
        for (int j = d; j <= N; j += d)
            cnt[d] += a[j];

    // f[d] = GCDがちょうど d であるペア数
    // メビウス関数的な包除: f[d] = C(cnt[d],2) - sum f[kd] for k>=2
    vector<ll> f(N + 1, 0);
    for (int d = N; d >= 1; d--) {
        f[d] = cnt[d] * (cnt[d] - 1) / 2;
        for (int k = 2 * d; k <= N; k += d)
            f[d] -= f[k];
    }

    // 例: GCD=1のペア数(互いに素なペア数)を出力
    cout << f[1] << endl;
    return 0;
}
