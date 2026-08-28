// Binary Indexed Tree (Fenwick Tree)
// 一点加算・区間和 / 転倒数の計算
// 計算量: 更新 O(log N), クエリ O(log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: BIT ===
template<typename T>
struct BIT {
    int n;
    vector<T> dat;
    BIT(int n) : n(n), dat(n + 1, 0) {}

    // a[i] += x (1-indexed)
    void add(int i, T x){
        for(; i <= n; i += i & (-i)) dat[i] += x;
    }

    // a[1] + ... + a[i] (1-indexed)
    T sum(int i){
        T s = 0;
        for(; i > 0; i -= i & (-i)) s += dat[i];
        return s;
    }

    // a[l] + ... + a[r] (1-indexed, 閉区間)
    T sum(int l, int r){
        return sum(r) - sum(l - 1);
    }

    // 累積和が k 以上になる最小の位置を返す (1-indexed)
    // 前提: 各要素が非負
    int lower_bound(T k){
        int pos = 0;
        for(int pw = 1 << __lg(n); pw > 0; pw >>= 1){
            if(pos + pw <= n && dat[pos + pw] < k){
                pos += pw;
                k -= dat[pos];
            }
        }
        return pos + 1;
    }
};

// === 例題: 転倒数 ===
// https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_5_D
// 数列の転倒数 = i < j かつ a[i] > a[j] であるペアの数
int main(){
    int N;
    cin >> N;
    vector<int> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];

    // 座標圧縮
    vector<int> sorted_a = a;
    sort(sorted_a.begin(), sorted_a.end());
    sorted_a.erase(unique(sorted_a.begin(), sorted_a.end()), sorted_a.end());
    for(int i = 0; i < N; i++){
        a[i] = lower_bound(sorted_a.begin(), sorted_a.end(), a[i]) - sorted_a.begin() + 1;
    }

    BIT<long long> bit(N);
    long long inv = 0;
    for(int i = 0; i < N; i++){
        inv += i - bit.sum(a[i]); // a[i]以下の数の個数を引く
        bit.add(a[i], 1);
    }
    cout << inv << endl;
    return 0;
}
