// ModInt + 行列累乗でフィボナッチ数
// [[1,1],[1,0]]^N の (0,0) 成分 = F(N)
// 計算量: O(K^3 log N), K=2
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1e9 + 7;

struct mint {
    long long v;
    mint(long long v = 0) : v((v % MOD + MOD) % MOD) {}
    mint operator+(mint o) const { return mint(v + o.v); }
    mint operator*(mint o) const { return mint(v * o.v); }
    friend ostream& operator<<(ostream& os, mint m) { return os << m.v; }
};

using Mat = vector<vector<mint>>;

Mat mul(const Mat& A, const Mat& B) {
    int n = A.size();
    Mat C(n, vector<mint>(n));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < n; j++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
    return C;
}

Mat mat_pow(Mat A, long long p) {
    int n = A.size();
    Mat R(n, vector<mint>(n));
    for (int i = 0; i < n; i++) R[i][i] = 1;
    while (p > 0) {
        if (p & 1) R = mul(R, A);
        A = mul(A, A);
        p >>= 1;
    }
    return R;
}

int main() {
    long long N;
    cin >> N;
    if (N <= 1) { cout << 1 << endl; return 0; }
    Mat A = {{mint(1), mint(1)}, {mint(1), mint(0)}};
    Mat R = mat_pow(A, N);
    cout << R[0][0] << endl;
    return 0;
}
