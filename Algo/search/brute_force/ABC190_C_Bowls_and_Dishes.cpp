// ABC190 C - Bowls and Dishes
// https://atcoder.jp/contests/abc190/tasks/abc190_c
// K人がそれぞれ2択。ビット全探索で最大条件充足数を求める。
// 計算量: O(2^K * (K + M))
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M;
    cin >> N >> M;
    vector<int> A(M), B(M);
    for (int i = 0; i < M; i++) cin >> A[i] >> B[i];
    int K;
    cin >> K;
    vector<int> C(K), D(K);
    for (int i = 0; i < K; i++) cin >> C[i] >> D[i];

    int ans = 0;
    for (int bit = 0; bit < (1 << K); bit++) {
        vector<bool> on(N + 1, false);
        for (int i = 0; i < K; i++) {
            if (bit >> i & 1) on[D[i]] = true;
            else on[C[i]] = true;
        }
        int cnt = 0;
        for (int i = 0; i < M; i++)
            if (on[A[i]] && on[B[i]]) cnt++;
        ans = max(ans, cnt);
    }
    cout << ans << endl;
    return 0;
}
