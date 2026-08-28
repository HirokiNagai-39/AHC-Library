// 行列累乗テンプレート
// 線形漸化式 a[n] = c1*a[n-1] + c2*a[n-2] + ... + ck*a[n-k] を O(K^3 log N) で計算
// 例: フィボナッチ F(N) = F(N-1) + F(N-2)
// 計算量: O(K^3 log N)
#include <bits/stdc++.h>
using namespace std;

using ll = long long;
static constexpr ll MOD = 1e9 + 7;

using Mat = vector<vector<ll>>;

Mat mat_mul(const Mat& A, const Mat& B) {
    int n = A.size(), m = B[0].size(), k = B.size();
    Mat C(n, vector<ll>(m, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < k; j++) if (A[i][j])
            for (int l = 0; l < m; l++)
                C[i][l] = (C[i][l] + A[i][j] * B[j][l]) % MOD;
    return C;
}

Mat mat_pow(Mat A, ll p) {
    int n = A.size();
    Mat R(n, vector<ll>(n, 0));
    for (int i = 0; i < n; i++) R[i][i] = 1;
    while (p > 0) {
        if (p & 1) R = mat_mul(R, A);
        A = mat_mul(A, A);
        p >>= 1;
    }
    return R;
}

int main() {
    ll N;
    cin >> N;
    // フィボナッチ: [F(n+1), F(n)] = [[1,1],[1,0]]^n * [1, 0]
    if (N <= 1) { cout << 1 << endl; return 0; }
    Mat A = {{1, 1}, {1, 0}};
    Mat R = mat_pow(A, N);
    cout << R[0][0] << endl;
    return 0;
}
