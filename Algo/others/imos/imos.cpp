// いもす法 (累積和の逆操作で区間加算を高速化)
// 例題: ABC183 D - Water Heater
// https://atcoder.jp/contests/abc183/tasks/abc183_d
// 計算量: O(N + T)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: いもす法 (1次元) ===
// [l, r) に +v を加算 → 最後にまとめて累積和
struct Imos1D {
    int n;
    vector<long long> dat;
    Imos1D(int n) : n(n), dat(n + 1, 0) {}

    void add(int l, int r, long long v){
        dat[l] += v;
        dat[r] -= v;
    }

    void build(){
        for(int i = 0; i < n; i++) dat[i + 1] += dat[i];
    }

    long long operator[](int i) const { return dat[i]; }
};

// === ライブラリ: いもす法 (2次元) ===
struct Imos2D {
    int H, W;
    vector<vector<long long>> dat;
    Imos2D(int H, int W) : H(H), W(W), dat(H + 1, vector<long long>(W + 1, 0)) {}

    // [r1, r2) x [c1, c2) に +v
    void add(int r1, int c1, int r2, int c2, long long v){
        dat[r1][c1] += v;
        dat[r1][c2] -= v;
        dat[r2][c1] -= v;
        dat[r2][c2] += v;
    }

    void build(){
        for(int i = 0; i <= H; i++)
            for(int j = 0; j < W; j++)
                dat[i][j + 1] += dat[i][j];
        for(int j = 0; j <= W; j++)
            for(int i = 0; i < H; i++)
                dat[i + 1][j] += dat[i][j];
    }

    long long get(int r, int c) const { return dat[r][c]; }
};

// === 例題: ABC183 D ===
// N人が [S_i, T_i) の時間帯に湯を P_i 使う. 容量W以下に収まるか
int main(){
    int N;
    long long W;
    cin >> N >> W;

    static constexpr int T = 200001;
    Imos1D imos(T);

    for(int i = 0; i < N; i++){
        int s, t;
        long long p;
        cin >> s >> t >> p;
        imos.add(s, t, p);
    }
    imos.build();

    for(int i = 0; i < T; i++){
        if(imos[i] > W){
            cout << "No" << endl;
            return 0;
        }
    }
    cout << "Yes" << endl;
    return 0;
}
