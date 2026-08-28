// ABC014 C - AtColor
// https://atcoder.jp/contests/abc014/tasks/abc014_3
// N個の区間[a,b]を塗る。最大重なり数。1Dいもす法。
// 計算量: O(N + max_val)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    static constexpr int MAXV = 1000002;
    vector<int> imos(MAXV, 0);
    for (int i = 0; i < N; i++) {
        int a, b;
        cin >> a >> b;
        imos[a]++;
        imos[b + 1]--;
    }
    int ans = 0, cur = 0;
    for (int i = 0; i < MAXV; i++) {
        cur += imos[i];
        ans = max(ans, cur);
    }
    cout << ans << endl;
    return 0;
}
