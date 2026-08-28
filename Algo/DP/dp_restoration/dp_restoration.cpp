// DP復元 (最適解の構成を復元)
// 例題: ABC211 D, ABC271 D 等
// DP値だけでなく, どの遷移を選んだかを記録して復元する
// 計算量: DP本体と同じ
#include <bits/stdc++.h>
using namespace std;

// ============================================================
// パターン1: ナップサックDP復元
// 選んだ品物のリストを返す
// ============================================================
vector<int> knapsack_restore(const vector<int>& w, const vector<int>& v, int W){
    int n = w.size();
    vector<vector<int>> dp(n + 1, vector<int>(W + 1, 0));
    for(int i = 0; i < n; i++){
        for(int j = 0; j <= W; j++){
            dp[i+1][j] = dp[i][j];
            if(j >= w[i]) dp[i+1][j] = max(dp[i+1][j], dp[i][j - w[i]] + v[i]);
        }
    }
    // 復元
    vector<int> selected;
    int j = W;
    for(int i = n; i > 0; i--){
        if(dp[i][j] != dp[i-1][j]){
            selected.push_back(i - 1); // 品物i-1を選んだ
            j -= w[i-1];
        }
    }
    reverse(selected.begin(), selected.end());
    return selected;
}

// ============================================================
// パターン2: 経路DP復元 (グリッド)
// 右 or 下に移動して最大スコアの経路を復元
// ============================================================
string grid_dp_restore(const vector<vector<int>>& grid){
    int H = grid.size(), W = grid[0].size();
    vector<vector<long long>> dp(H, vector<long long>(W, 0));
    dp[0][0] = grid[0][0];
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            if(i == 0 && j == 0) continue;
            dp[i][j] = -1e18;
            if(i > 0) dp[i][j] = max(dp[i][j], dp[i-1][j]);
            if(j > 0) dp[i][j] = max(dp[i][j], dp[i][j-1]);
            dp[i][j] += grid[i][j];
        }
    }
    // 復元 (右下から左上へ)
    string path;
    int i = H - 1, j = W - 1;
    while(i > 0 || j > 0){
        if(i == 0){ path += 'R'; j--; }
        else if(j == 0){ path += 'D'; i--; }
        else if(dp[i-1][j] >= dp[i][j-1]){ path += 'D'; i--; }
        else { path += 'R'; j--; }
    }
    reverse(path.begin(), path.end());
    return path;
}

// ============================================================
// パターン3: 汎用 prev 配列による復元
// prev[state] = 1つ前の状態, を記録しておく
// ============================================================

// === 例題: ナップサック復元 ===
int main(){
    int N, W;
    cin >> N >> W;
    vector<int> w(N), v(N);
    for(int i = 0; i < N; i++) cin >> w[i] >> v[i];

    auto selected = knapsack_restore(w, v, W);

    cout << "Selected items:";
    for(int i : selected) cout << " " << i;
    cout << endl;
    return 0;
}
