// ABC258 E - Packing Potatoes
// https://atcoder.jp/contests/abc258/tasks/abc258_e
// 重さが周期Nで繰り返す。各箱にX以上になるまで詰める。K番目の箱の個数。
// 二分探索で1箱分の個数を求め、ダブリングで高速化。
// 計算量: O(N log N + Q log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    long long N, Q, X;
    cin >> N >> Q >> X;
    vector<long long> W(N);
    for (auto& w : W) cin >> w;

    long long total = 0;
    for (auto w : W) total += w;
    vector<long long> pre(2 * N + 1, 0);
    for (int i = 0; i < 2 * N; i++) pre[i+1] = pre[i] + W[i % N];

    // nxt[i]: 位置iから始めて1箱詰めたら次はどの位置(mod N)か、cnt[i]: 何個入るか
    vector<int> nxt(N);
    vector<long long> cnt(N);
    for (int i = 0; i < N; i++) {
        // 周期全体の和がsum。最低 full = X/total 周 + 残り
        long long full = X / total;
        long long rem = X - full * total;
        // pre[i]からrem以上になる最小j
        long long base = pre[i];
        // 区間 [i, i+2N) で累積和から二分探索
        int lo = 0, hi = N; // [i, i+lo..i+hi)
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (pre[i + mid + 1] - pre[i] >= rem) hi = mid;
            else lo = mid + 1;
        }
        long long items = full * N + lo + 1;
        cnt[i] = items;
        nxt[i] = (i + items) % N;
    }

    // ダブリングで K 番目の箱
    int LOG = 61;
    vector<vector<int>> to(LOG, vector<int>(N));
    for (int i = 0; i < N; i++) to[0][i] = nxt[i];
    for (int k = 1; k < LOG; k++)
        for (int i = 0; i < N; i++)
            to[k][i] = to[k-1][to[k-1][i]];

    while (Q--) {
        long long K;
        cin >> K;
        K--; // 0-indexed
        int cur = 0;
        for (int k = LOG - 1; k >= 0; k--)
            if (K >> k & 1) cur = to[k][cur];
        cout << cnt[cur] << "\n";
    }
    return 0;
}
