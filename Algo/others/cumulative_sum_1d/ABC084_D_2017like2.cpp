// ABC084 D - 2017-like Number (累積和視点)
// https://atcoder.jp/contests/abc084/tasks/abc084_d
// 篩で2017-like判定 + 累積和で区間クエリに応答。
// 計算量: O(N log log N + Q)
#include <bits/stdc++.h>
using namespace std;

int main() {
    static constexpr int MAXN = 100001;
    vector<bool> is_prime(MAXN, true);
    is_prime[0] = is_prime[1] = false;
    for (int i = 2; (long long)i * i < MAXN; i++)
        if (is_prime[i])
            for (int j = i*i; j < MAXN; j += i) is_prime[j] = false;

    // 累積和: cum[i] = [1,i]中の2017-like数の個数
    vector<int> cum(MAXN, 0);
    for (int i = 2; i < MAXN; i++) {
        cum[i] = cum[i-1];
        if (i >= 3 && i % 2 == 1 && is_prime[i] && is_prime[(i+1)/2])
            cum[i]++;
    }

    int Q;
    cin >> Q;
    while (Q--) {
        int l, r;
        cin >> l >> r;
        cout << cum[r] - cum[l-1] << "\n";
    }
    return 0;
}
