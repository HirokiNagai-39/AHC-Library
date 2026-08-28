// 最長増加部分列 (LIS: Longest Increasing Subsequence)
// https://atcoder.jp/contests/dp/tasks/dp_q (EDPC Q - Flowers は重み付きLIS)
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: LIS ===
// 狭義単調増加 (strict): lower_bound
// 広義単調増加 (non-strict): upper_bound
int lis_length(const vector<int>& a){
    vector<int> dp; // dp[i] = 長さ(i+1)のLISの末尾の最小値
    for(int x : a){
        auto it = lower_bound(dp.begin(), dp.end(), x); // strictの場合
        if(it == dp.end()) dp.push_back(x);
        else *it = x;
    }
    return dp.size();
}

// LIS復元付き
vector<int> lis_restore(const vector<int>& a){
    int n = a.size();
    vector<int> dp, pos(n), from(n, -1);
    // pos[i] = a[i]がdpの何番目に入ったか
    for(int i = 0; i < n; i++){
        auto it = lower_bound(dp.begin(), dp.end(), a[i]);
        int k = it - dp.begin();
        if(it == dp.end()) dp.push_back(a[i]);
        else *it = a[i];
        pos[i] = k;
        // 復元用: 1つ前の位置を探す
    }
    // 復元
    int len = dp.size();
    vector<int> res(len);
    int cur = len - 1;
    for(int i = n - 1; i >= 0 && cur >= 0; i--){
        if(pos[i] == cur){
            res[cur] = a[i];
            cur--;
        }
    }
    return res;
}

// === 例題: 単純なLIS長を求める ===
int main(){
    int N;
    cin >> N;
    vector<int> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];
    cout << lis_length(a) << endl;
    return 0;
}
