// ABC125 C - GCD on Blackboard
// https://atcoder.jp/contests/abc125/tasks/abc125_c
// 1つ除いたときのGCD最大値。前方・後方GCDを前計算。
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    vector<long long> pre(N + 1, 0), suf(N + 1, 0);
    for (int i = 0; i < N; i++) pre[i + 1] = __gcd(pre[i], A[i]);
    for (int i = N - 1; i >= 0; i--) suf[i] = __gcd(suf[i + 1], A[i]);

    long long ans = 0;
    for (int i = 0; i < N; i++)
        ans = max(ans, __gcd(pre[i], suf[i + 1]));
    cout << ans << endl;
    return 0;
}
