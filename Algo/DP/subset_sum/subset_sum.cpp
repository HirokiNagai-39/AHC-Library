// 部分和問題 / コイン問題
// 例題: EDPC C - Vacation, 部分和判定, コインの組み合わせ数
// 計算量: O(NS) (Sは目標値)
#include <bits/stdc++.h>
using namespace std;

// ============================================================
// パターン1: 部分和判定
// N個の整数から部分集合を選び, 和がちょうどSにできるか
// 計算量: O(NS)
// ============================================================
bool subset_sum_possible(const vector<int>& a, int S){
    int n = a.size();
    vector<bool> dp(S + 1, false);
    dp[0] = true;
    for(int i = 0; i < n; i++){
        // 逆順に回す (各要素は1回まで)
        for(int j = S; j >= a[i]; j--){
            if(dp[j - a[i]]) dp[j] = true;
        }
    }
    return dp[S];
}

// ============================================================
// パターン2: 部分和の個数 (mod付き)
// 和がちょうどSになる部分集合の数
// 計算量: O(NS)
// ============================================================
long long subset_sum_count(const vector<int>& a, int S, long long MOD){
    int n = a.size();
    vector<long long> dp(S + 1, 0);
    dp[0] = 1;
    for(int i = 0; i < n; i++){
        for(int j = S; j >= a[i]; j--){
            dp[j] = (dp[j] + dp[j - a[i]]) % MOD;
        }
    }
    return dp[S];
}

// ============================================================
// パターン3: コイン問題 (個数制限なし)
// N種類のコインで金額Sを支払う最小枚数
// 計算量: O(NS)
// ============================================================
int coin_min(const vector<int>& coins, int S){
    static constexpr int INF = 1e9;
    vector<int> dp(S + 1, INF);
    dp[0] = 0;
    for(int c : coins){
        for(int j = c; j <= S; j++){ // 順方向 (個数制限なし)
            if(dp[j - c] < INF) dp[j] = min(dp[j], dp[j - c] + 1);
        }
    }
    return dp[S] == INF ? -1 : dp[S];
}

// ============================================================
// パターン4: コイン問題 (組み合わせ数, 個数制限なし)
// N種類のコインで金額Sを支払う方法の数
// 計算量: O(NS)
// ============================================================
long long coin_count(const vector<int>& coins, int S){
    vector<long long> dp(S + 1, 0);
    dp[0] = 1;
    for(int c : coins){
        for(int j = c; j <= S; j++){
            dp[j] += dp[j - c];
        }
    }
    return dp[S];
}

// === 例題: 部分和判定 ===
int main(){
    int N, S;
    cin >> N >> S;
    vector<int> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];

    if(subset_sum_possible(a, S)){
        cout << "Yes" << endl;
    } else {
        cout << "No" << endl;
    }
    return 0;
}
