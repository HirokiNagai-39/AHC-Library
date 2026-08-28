// 二次元累積和
// 例題: ABC005 D, ABC086 C 等 (矩形領域の和)
// 計算量: 前処理 O(HW), クエリ O(1)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 二次元累積和 ===
template<typename T>
struct CumSum2D {
    int H, W;
    vector<vector<T>> sum;

    CumSum2D(const vector<vector<T>>& grid){
        H = grid.size();
        W = grid[0].size();
        sum.assign(H + 1, vector<T>(W + 1, 0));
        for(int i = 0; i < H; i++){
            for(int j = 0; j < W; j++){
                sum[i+1][j+1] = grid[i][j] + sum[i][j+1] + sum[i+1][j] - sum[i][j];
            }
        }
    }

    // [r1, r2) x [c1, c2) の矩形和
    T query(int r1, int c1, int r2, int c2){
        return sum[r2][c2] - sum[r1][c2] - sum[r2][c1] + sum[r1][c1];
    }
};

// === 例題: グリッド上の矩形和クエリ ===
int main(){
    int H, W, Q;
    cin >> H >> W >> Q;
    vector<vector<long long>> grid(H, vector<long long>(W));
    for(int i = 0; i < H; i++)
        for(int j = 0; j < W; j++)
            cin >> grid[i][j];

    CumSum2D<long long> cs(grid);

    while(Q--){
        int r1, c1, r2, c2;
        cin >> r1 >> c1 >> r2 >> c2;
        // 0-indexed, [r1,r2) x [c1,c2) → r2,c2は含まない
        cout << cs.query(r1, c1, r2, c2) << "\n";
    }
    return 0;
}
