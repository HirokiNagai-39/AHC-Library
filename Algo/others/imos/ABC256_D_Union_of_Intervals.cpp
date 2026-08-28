// ABC256 D - Union of Intervals
// https://atcoder.jp/contests/abc256/tasks/abc256_d
// N個の区間の和集合を求める。いもす or ソートでマージ。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<pair<int,int>> segs(N);
    for (auto& [l, r] : segs) cin >> l >> r;
    sort(segs.begin(), segs.end());

    vector<pair<int,int>> merged;
    int cl = segs[0].first, cr = segs[0].second;
    for (int i = 1; i < N; i++) {
        if (segs[i].first <= cr) {
            cr = max(cr, segs[i].second);
        } else {
            merged.push_back({cl, cr});
            cl = segs[i].first;
            cr = segs[i].second;
        }
    }
    merged.push_back({cl, cr});

    for (auto& [l, r] : merged)
        cout << l << " " << r << "\n";
    return 0;
}
