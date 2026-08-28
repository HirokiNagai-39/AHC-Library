// ダブリングでパス上の最大値 (Functional Graph)
// 各頂点から2^k回移動した先と、その間の辺の最大値を管理。
// 計算量: O(N log K) 前処理, O(log K) クエリ
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long K;
    cin >> N >> K;
    vector<int> nxt(N);
    vector<long long> w(N); // 辺の重み
    for (int i = 0; i < N; i++) cin >> nxt[i] >> w[i];

    int LOG = 60;
    vector<vector<int>> to(LOG, vector<int>(N));
    vector<vector<long long>> mx(LOG, vector<long long>(N, 0));

    // 初期化: 2^0 = 1回の移動
    for (int i = 0; i < N; i++) {
        to[0][i] = nxt[i];
        mx[0][i] = w[i];
    }
    // ダブリング前処理
    for (int k = 1; k < LOG; k++)
        for (int i = 0; i < N; i++) {
            to[k][i] = to[k-1][to[k-1][i]];
            mx[k][i] = max(mx[k-1][i], mx[k-1][to[k-1][i]]);
        }

    // クエリ: 頂点0からK回移動したパス上の最大値
    int cur = 0;
    long long ans = 0;
    for (int k = 0; k < LOG; k++) {
        if (K >> k & 1) {
            ans = max(ans, mx[k][cur]);
            cur = to[k][cur];
        }
    }
    cout << ans << endl;
    return 0;
}
