// 尺取り法 (Two Pointers / Sliding Window)
// 例題: ABC032 C - 列
// https://atcoder.jp/contests/abc032/tasks/abc032_c
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

// === 尺取り法テンプレート ===
// 条件を満たす最長/最短の連続部分列を求める
// [l, r) の区間を管理

// === 例題: ABC032 C ===
// 数列 S の連続部分列で, 積が K 以下のものの最大長
int main(){
    int N;
    long long K;
    cin >> N >> K;
    vector<long long> S(N);
    for(int i = 0; i < N; i++) cin >> S[i];

    // 0が含まれる場合は全体が答え
    for(int i = 0; i < N; i++){
        if(S[i] == 0){
            cout << N << endl;
            return 0;
        }
    }

    int ans = 0;
    long long prod = 1;
    int r = 0;

    for(int l = 0; l < N; l++){
        // rを右に伸ばす
        while(r < N && prod * S[r] <= K){
            prod *= S[r];
            r++;
        }
        ans = max(ans, r - l);
        // lを1つ右に
        if(r == l){
            r++;
        } else {
            prod /= S[l];
        }
    }
    cout << ans << endl;
    return 0;
}
