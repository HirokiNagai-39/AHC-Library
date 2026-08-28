// 期待値DP
// 例題: EDPC J - Sushi
// https://atcoder.jp/contests/dp/tasks/dp_j
// N皿の寿司. 各皿に1~3個. ランダムに皿を選び1個食べる.
// 全部食べ終わるまでの操作回数の期待値
// 計算量: O(N^3)
#include <bits/stdc++.h>
using namespace std;

// === 期待値DPの考え方 ===
// E[状態] = 1 + Σ (遷移確率 * E[遷移先])
// 「今の状態から終了までの期待値」を未知数として方程式を立てる
// 多くの場合, 状態を適切に圧縮することがポイント

int main(){
    int N;
    cin >> N;
    vector<int> a(N);
    int c1 = 0, c2 = 0, c3 = 0; // 寿司が1,2,3個の皿の数
    for(int i = 0; i < N; i++){
        cin >> a[i];
        if(a[i] == 1) c1++;
        else if(a[i] == 2) c2++;
        else c3++;
    }

    // dp[i][j][k] = 寿司が1個の皿がi個, 2個がj個, 3個がk個 の状態から
    //               全部食べ終わるまでの期待操作回数
    // dp[0][0][0] = 0
    // dp[i][j][k] = N/(i+j+k) + (i/(i+j+k))*dp[i-1][j][k]
    //             + (j/(i+j+k))*dp[i+1][j-1][k]
    //             + (k/(i+j+k))*dp[i][j+1][k-1]
    // 整理: dp[i][j][k] = N/(i+j+k) + i*dp[i-1][j][k]/(i+j+k)
    //                    + j*dp[i+1][j-1][k]/(i+j+k)
    //                    + k*dp[i][j+1][k-1]/(i+j+k)
    // ※ 空の皿を選んだ場合は何も起こらない → 有効な皿で割り直す

    vector<vector<vector<double>>> dp(N+1, vector<vector<double>>(N+1, vector<double>(N+1, 0)));

    for(int k = 0; k <= N; k++){
        for(int j = 0; j <= N - k; j++){
            for(int i = 0; i <= N - j - k; i++){
                if(i == 0 && j == 0 && k == 0) continue;
                double s = i + j + k;
                double val = (double)N / s;
                if(i > 0) val += (double)i / s * dp[i-1][j][k];
                if(j > 0) val += (double)j / s * dp[i+1][j-1][k];
                if(k > 0) val += (double)k / s * dp[i][j+1][k-1];
                dp[i][j][k] = val;
            }
        }
    }

    printf("%.10f\n", dp[c1][c2][c3]);
    return 0;
}
