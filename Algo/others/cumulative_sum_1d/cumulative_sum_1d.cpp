// 一次元累積和
// 例題: ABC122 C - GeT AC
// https://atcoder.jp/contests/abc122/tasks/abc122_c
// 計算量: 前処理 O(N), クエリ O(1)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 一次元累積和 ===
// S[i] = a[0] + a[1] + ... + a[i-1]
// a[l] + ... + a[r-1] = S[r] - S[l]
template<typename T>
struct CumSum1D {
    vector<T> sum;
    CumSum1D(const vector<T>& a){
        int n = a.size();
        sum.resize(n + 1, 0);
        for(int i = 0; i < n; i++) sum[i+1] = sum[i] + a[i];
    }
    // [l, r) の和
    T query(int l, int r){ return sum[r] - sum[l]; }
};

// === 例題: ABC122 C ===
// 文字列Sの[l,r]に含まれる "AC" の個数
int main(){
    int N, Q;
    cin >> N >> Q;
    string S;
    cin >> S;

    // a[i] = S[i..i+1] が "AC" なら 1
    vector<int> a(N, 0);
    for(int i = 0; i + 1 < N; i++){
        if(S[i] == 'A' && S[i+1] == 'C') a[i] = 1;
    }
    CumSum1D<int> cs(a);

    while(Q--){
        int l, r;
        cin >> l >> r;
        l--; r--; // 0-indexed
        cout << cs.query(l, r) << "\n";
    }
    return 0;
}
