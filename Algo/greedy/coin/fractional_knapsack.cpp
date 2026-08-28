// 分数ナップサック (Fractional Knapsack)
// 品物を分割可能。単価(価値/重さ)の高い順に詰める。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    double W;
    cin >> N >> W;
    vector<double> v(N), w(N);
    for (int i = 0; i < N; i++) cin >> v[i] >> w[i];

    // 単価でソート (降順)
    vector<int> idx(N);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return v[a] / w[a] > v[b] / w[b];
    });

    double total = 0, remain = W;
    for (int i : idx) {
        if (remain <= 0) break;
        double take = min(remain, w[i]);
        total += take * (v[i] / w[i]);
        remain -= take;
    }
    cout << fixed << setprecision(10) << total << endl;
    return 0;
}
