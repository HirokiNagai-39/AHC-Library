// 個数制限なしナップサック
// 各品物を何個でも使える場合
// 例題: EDPC C に類似 / 典型
// 計算量: O(NW)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 個数制限なしナップサック ===
// 品物i: 重さw[i], 価値v[i], 何個でも使える
// 重さW以下での最大価値
long long unbounded_knapsack(const vector<int>& w, const vector<long long>& v, int W){
    int n = w.size();
    vector<long long> dp(W + 1, 0);
    for(int i = 0; i < n; i++){
        // 順方向に回す → 同じ品物を何度でも使える
        for(int j = w[i]; j <= W; j++){
            dp[j] = max(dp[j], dp[j - w[i]] + v[i]);
        }
    }
    return dp[W];
}

// === ライブラリ: 個数制限付きナップサック ===
// 品物i: 重さw[i], 価値v[i], 最大c[i]個
// 二進展開でO(NW log C)に高速化
long long bounded_knapsack(const vector<int>& w, const vector<long long>& v,
                           const vector<int>& c, int W){
    int n = w.size();
    vector<long long> dp(W + 1, 0);
    for(int i = 0; i < n; i++){
        // c[i]個を二進展開: 1, 2, 4, ..., 残り
        int rem = c[i];
        for(int k = 1; rem > 0; k <<= 1){
            int take = min(k, rem);
            rem -= take;
            int wi = w[i] * take;
            long long vi = v[i] * take;
            // 0-1ナップサック (逆順)
            for(int j = W; j >= wi; j--){
                dp[j] = max(dp[j], dp[j - wi] + vi);
            }
        }
    }
    return dp[W];
}

// === 例題 ===
int main(){
    int N, W;
    cin >> N >> W;
    vector<int> w(N);
    vector<long long> v(N);
    for(int i = 0; i < N; i++) cin >> w[i] >> v[i];

    cout << unbounded_knapsack(w, v, W) << endl;
    return 0;
}
