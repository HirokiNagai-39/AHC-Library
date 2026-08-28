// ABC083 C - Multiple Gift
// https://atcoder.jp/contests/abc083/tasks/arc088_a
// X以上Y以下で前の数の倍数になる最長列。倍々(x2)で伸ばす。
// 計算量: O(log(Y/X))
#include <bits/stdc++.h>
using namespace std;

int main() {
    long long X, Y;
    cin >> X >> Y;
    int cnt = 0;
    long long cur = X;
    while (cur <= Y) {
        cnt++;
        cur *= 2;
    }
    cout << cnt << endl;
    return 0;
}
