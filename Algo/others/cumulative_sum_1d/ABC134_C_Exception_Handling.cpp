// ABC134 C - Exception Handling
// https://atcoder.jp/contests/abc134/tasks/abc134_c
// i番目を除いた最大値。前方最大・後方最大の前計算。
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<int> A(N);
    for (auto& a : A) cin >> a;

    vector<int> pre(N + 1, 0), suf(N + 1, 0);
    for (int i = 0; i < N; i++) pre[i+1] = max(pre[i], A[i]);
    for (int i = N-1; i >= 0; i--) suf[i] = max(suf[i+1], A[i]);

    for (int i = 0; i < N; i++)
        cout << max(pre[i], suf[i+1]) << "\n";
    return 0;
}
