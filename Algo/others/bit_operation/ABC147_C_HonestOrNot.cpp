// ABC147 C - HonestOrUnkind
// https://atcoder.jp/contests/abc147/tasks/abc147_c
// N人の証言。ビット全探索で正直者の最大数。矛盾がないか判定。
// 計算量: O(2^N * M) M=証言数
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<vector<pair<int,int>>> testimony(N);
    for (int i = 0; i < N; i++) {
        int A;
        cin >> A;
        for (int j = 0; j < A; j++) {
            int x, y;
            cin >> x >> y;
            x--;
            testimony[i].push_back({x, y});
        }
    }

    int ans = 0;
    for (int bit = 0; bit < (1 << N); bit++) {
        bool ok = true;
        for (int i = 0; i < N && ok; i++) {
            if (!(bit >> i & 1)) continue; // i は不親切 → 証言無視
            for (auto [x, y] : testimony[i]) {
                int actual = (bit >> x & 1);
                if (actual != y) { ok = false; break; }
            }
        }
        if (ok) ans = max(ans, __builtin_popcount(bit));
    }
    cout << ans << endl;
    return 0;
}
