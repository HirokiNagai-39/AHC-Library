// ABC246 D - 2-variable Function
// https://atcoder.jp/contests/abc246/tasks/abc246_d
// a^3 + a^2*b + a*b^2 + b^3 >= N の最小値。aを全探索、bを二分探索。
// 計算量: O(N^(1/3) * log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    long long N;
    cin >> N;
    long long ans = LLONG_MAX;
    // a, b >= 0, a^3+a^2*b+a*b^2+b^3 = (a+b)(a^2+b^2) >= N
    for (long long a = 0; a * a * a <= 2 * N; a++) {
        // 二分探索でbを探す
        long long lo = 0, hi = 1000001;
        while (hi - lo > 1) {
            long long b = (lo + hi) / 2;
            long long val = a*a*a + a*a*b + a*b*b + b*b*b;
            if (val >= N) hi = b;
            else lo = b;
        }
        long long val = a*a*a + a*a*hi + a*hi*hi + hi*hi*hi;
        ans = min(ans, val);
    }
    cout << ans << endl;
    return 0;
}
