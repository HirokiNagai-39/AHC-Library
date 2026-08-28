// 座標圧縮
// 大きな値の座標を 0,1,2,... に変換
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 座標圧縮 ===
template<typename T>
struct Compress {
    vector<T> vals;

    void add(T x){ vals.push_back(x); }

    void build(){
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());
    }

    // 圧縮後の値 (0-indexed)
    int get(T x){
        return lower_bound(vals.begin(), vals.end(), x) - vals.begin();
    }

    // 圧縮前の値
    T rev(int i){ return vals[i]; }

    int size(){ return vals.size(); }
};

// === 例題: ABC036 C - 座圧 ===
// https://atcoder.jp/contests/abc036/tasks/abc036_c
int main(){
    int N;
    cin >> N;
    vector<int> a(N);
    Compress<int> comp;
    for(int i = 0; i < N; i++){
        cin >> a[i];
        comp.add(a[i]);
    }
    comp.build();
    for(int i = 0; i < N; i++){
        cout << comp.get(a[i]) << "\n";
    }
    return 0;
}
