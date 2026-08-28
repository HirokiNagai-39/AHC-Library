// ABC185 C - Duodecim Ferra
// https://atcoder.jp/contests/abc185/tasks/abc185_c
// 長さLの鉄棒を12個に分割する方法数 = C(L-1, 11)
// 仕切り11本をL-1箇所から選ぶ
// 計算量: O(11)
#include <bits/stdc++.h>
using namespace std;

int main() {
    long long L;
    cin >> L;
    // C(L-1, 11) = (L-1)*(L-2)*...*(L-11) / 11!
    // L <= 200 なので溢れない(__int128 or 順に割る)
    // 11! = 39916800, L-1 <= 199 => 積は最大199*198*...*189 ~ 10^25 -> use __int128
    __int128 ans = 1;
    for (int i = 0; i < 11; i++) ans *= (L - 1 - i);
    long long fac = 1;
    for (int i = 1; i <= 11; i++) fac *= i;
    ans /= fac;
    cout << (long long)ans << endl;
    return 0;
}
