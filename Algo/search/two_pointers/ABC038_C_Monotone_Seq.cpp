// ABC038 C - 単調増加
// https://atcoder.jp/contests/abc038/tasks/abc038_c
// 狭義単調増加な連続部分列の個数。尺取り法。
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<int> A(N);
    for (auto& a : A) cin >> a;

    long long ans = 0;
    int l = 0;
    for (int r = 0; r < N; r++) {
        if (r > 0 && A[r] <= A[r-1]) l = r;
        ans += r - l + 1;
    }
    cout << ans << endl;
    return 0;
}
