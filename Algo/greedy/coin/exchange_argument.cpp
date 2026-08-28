// 交換論法 (Exchange Argument)
// 隣接する2要素を交換して損得を比較 → ソート順の根拠とする。
// 例: 仕事スケジューリング。締切d[i], 所要時間t[i]。遅延最大値を最小化。
// ソート順: t[i]昇順 (SPT rule)
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<int> d(N), t(N);
    for (int i = 0; i < N; i++) cin >> d[i] >> t[i];

    // 交換論法: i,jの順でi先 vs j先
    // i先: max(time+t[i]-d[i], time+t[i]+t[j]-d[j])
    // j先: max(time+t[j]-d[j], time+t[j]+t[i]-d[i])
    // d[i]-t[i]が大きい順(=締切に余裕が少ない方を後に回すと損)
    vector<int> idx(N);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return d[a] - t[a] < d[b] - t[b]; // 余裕が小さい順 = 締切が早い仕事を先に
    });

    long long time_now = 0, max_delay = 0;
    for (int i : idx) {
        time_now += t[i];
        max_delay = max(max_delay, time_now - d[i]);
    }
    cout << max_delay << endl;
    return 0;
}
