// ABC077 C - Snuke Festival (ARC084 A)
// https://atcoder.jp/contests/abc077/tasks/arc084_a
// 3層 A<B<C の組数。Bを固定し、A側でupper_bound、C側でlower_bound。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<long long> A(N), B(N), C(N);
    for (auto& a : A) cin >> a;
    for (auto& b : B) cin >> b;
    for (auto& c : C) cin >> c;
    sort(A.begin(), A.end());
    sort(B.begin(), B.end());
    sort(C.begin(), C.end());

    long long ans = 0;
    for (int i = 0; i < N; i++) {
        long long a_cnt = lower_bound(A.begin(), A.end(), B[i]) - A.begin();
        long long c_cnt = N - (upper_bound(C.begin(), C.end(), B[i]) - C.begin());
        ans += a_cnt * c_cnt;
    }
    cout << ans << endl;
    return 0;
}
