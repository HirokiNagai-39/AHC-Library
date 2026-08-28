// 区間被覆問題
// 点集合を与えられた区間で被覆する最小区間数。
// 点をソートし、貪欲に最も多くカバーする区間を選ぶ。
// 計算量: O(N log N + M log M)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M;
    cin >> N >> M;
    vector<int> points(N);
    for (auto& p : points) cin >> p;
    vector<pair<int,int>> intervals(M);
    for (auto& [l, r] : intervals) cin >> l >> r;

    sort(points.begin(), points.end());
    sort(intervals.begin(), intervals.end());

    int ans = 0, idx = 0, pi = 0;
    while (pi < N) {
        // points[pi]をカバーする区間のうち、右端最大のものを選ぶ
        int best_r = -1;
        while (idx < M && intervals[idx].first <= points[pi]) {
            best_r = max(best_r, intervals[idx].second);
            idx++;
        }
        if (best_r < points[pi]) {
            cout << -1 << endl; // 被覆不可
            return 0;
        }
        ans++;
        while (pi < N && points[pi] <= best_r) pi++;
    }
    cout << ans << endl;
    return 0;
}
