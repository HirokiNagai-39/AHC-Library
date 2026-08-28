// ABC103 D - Islands War
// https://atcoder.jp/contests/abc103/tasks/abc103_d
// 区間[a,b]を全て切断する最小の橋の除去数。
// 終端でソートし、貪欲に最も右で切る。
// 計算量: O(M log M)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M;
    cin >> N >> M;
    vector<pair<int,int>> segs(M);
    for (auto& [a, b] : segs) cin >> a >> b;
    // 終端でソート
    sort(segs.begin(), segs.end(), [](auto& a, auto& b) {
        return a.second < b.second;
    });

    int ans = 0, last_cut = -1;
    for (auto& [a, b] : segs) {
        if (last_cut <= a) {
            // まだこの区間を切断していない -> b-1 で切る
            last_cut = b;
            ans++;
        }
    }
    cout << ans << endl;
    return 0;
}
