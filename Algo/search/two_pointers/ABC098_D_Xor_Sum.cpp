// ABC098 D - Xor Sum 2 (ARC098 B)
// https://atcoder.jp/contests/abc098/tasks/arc098_b
// A[l]+...+A[r] = A[l] XOR ... XOR A[r] となる区間(l,r)の数。
// XOR=和 ⇔ 繰り上がりなし。尺取りで条件が単調。
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    long long ans = 0, sum = 0, xr = 0;
    int l = 0;
    for (int r = 0; r < N; r++) {
        sum += A[r];
        xr ^= A[r];
        while (sum != xr) {
            sum -= A[l];
            xr ^= A[l];
            l++;
        }
        ans += r - l + 1;
    }
    cout << ans << endl;
    return 0;
}
