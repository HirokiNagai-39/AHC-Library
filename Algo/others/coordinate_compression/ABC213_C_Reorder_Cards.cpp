// ABC213 C - Reorder Cards
// https://atcoder.jp/contests/abc213/tasks/abc213_c
// H*Wのグリッドで、空行・空列を詰める = 行列それぞれ座標圧縮。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int H, W, N;
    cin >> H >> W >> N;
    vector<int> R(N), C(N);
    for (int i = 0; i < N; i++) cin >> R[i] >> C[i];

    // 行の座圧
    vector<int> rs = R;
    sort(rs.begin(), rs.end());
    rs.erase(unique(rs.begin(), rs.end()), rs.end());

    // 列の座圧
    vector<int> cs = C;
    sort(cs.begin(), cs.end());
    cs.erase(unique(cs.begin(), cs.end()), cs.end());

    for (int i = 0; i < N; i++) {
        int r = lower_bound(rs.begin(), rs.end(), R[i]) - rs.begin() + 1;
        int c = lower_bound(cs.begin(), cs.end(), C[i]) - cs.begin() + 1;
        cout << r << " " << c << "\n";
    }
    return 0;
}
