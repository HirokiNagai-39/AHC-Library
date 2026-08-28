// ABC130 D - Enough Array
// https://atcoder.jp/contests/abc130/tasks/abc130_d
// 部分配列の和 >= K となる連続部分列の個数。尺取り法。
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long K;
    cin >> N >> K;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    long long ans = 0, sum = 0;
    int l = 0;
    for (int r = 0; r < N; r++) {
        sum += A[r];
        while (sum >= K) {
            // [l, r], [l, r+1], ..., [l, N-1] すべて条件を満たす
            ans += N - r;
            sum -= A[l];
            l++;
        }
    }
    cout << ans << endl;
    return 0;
}
