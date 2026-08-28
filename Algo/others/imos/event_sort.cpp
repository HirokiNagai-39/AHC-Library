// イベントソート (いもす法の代替)
// 開始/終了イベントをソートして走査。座標が大きい場合に有効。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<pair<long long, int>> events;
    for (int i = 0; i < N; i++) {
        long long l, r;
        cin >> l >> r;
        events.push_back({l, +1});   // 開始
        events.push_back({r + 1, -1}); // 終了
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
