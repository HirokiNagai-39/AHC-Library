// 二次元座標圧縮 + 面積計算
// N個の矩形の和面積を座圧+走査で求める。
// 計算量: O(N^2) (座圧後のグリッドサイズ)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<long long> x1(N), y1(N), x2(N), y2(N);
    for (int i = 0; i < N; i++) cin >> x1[i] >> y1[i] >> x2[i] >> y2[i];

    // 座標圧縮
    vector<long long> xs, ys;
    for (int i = 0; i < N; i++) {
        xs.push_back(x1[i]); xs.push_back(x2[i]);
        ys.push_back(y1[i]); ys.push_back(y2[i]);
    }
    sort(xs.begin(), xs.end()); xs.erase(unique(xs.begin(), xs.end()), xs.end());
    sort(ys.begin(), ys.end()); ys.erase(unique(ys.begin(), ys.end()), ys.end());

    int W = xs.size() - 1, H = ys.size() - 1;
    vector<vector<bool>> covered(W, vector<bool>(H, false));

    for (int k = 0; k < N; k++) {
        int lx = lower_bound(xs.begin(), xs.end(), x1[k]) - xs.begin();
        int rx = lower_bound(xs.begin(), xs.end(), x2[k]) - xs.begin();
        int ly = lower_bound(ys.begin(), ys.end(), y1[k]) - ys.begin();
        int ry = lower_bound(ys.begin(), ys.end(), y2[k]) - ys.begin();
        for (int i = lx; i < rx; i++)
            for (int j = ly; j < ry; j++)
                covered[i][j] = true;
    }

    long long area = 0;
    for (int i = 0; i < W; i++)
        for (int j = 0; j < H; j++)
            if (covered[i][j])
                area += (xs[i+1] - xs[i]) * (ys[j+1] - ys[j]);
    cout << area << endl;
    return 0;
}
