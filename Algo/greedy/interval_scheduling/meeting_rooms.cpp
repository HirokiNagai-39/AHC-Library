// 会議室問題 (必要な部屋数の最小値)
// いもす法 or イベントソートで重なりの最大数を求める。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<pair<int,int>> events; // {time, +1 or -1}
    for (int i = 0; i < N; i++) {
        int s, e;
        cin >> s >> e;
        events.push_back({s, 1});
        events.push_back({e, -1});
    }
    sort(events.begin(), events.end());

    int cur = 0, ans = 0;
    for (auto& [t, d] : events) {
        cur += d;
        ans = max(ans, cur);
    }
    cout << ans << endl;
    return 0;
}
