// ABC118 C - Monsters Battle Royale
// https://atcoder.jp/contests/abc118/tasks/abc118_c
// N個のモンスターが戦い、残る最小体力 = GCD(A1,...,AN)
// 計算量: O(N log max(A))
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    long long g = 0;
    for (int i = 0; i < N; i++) {
        long long a;
        cin >> a;
        g = __gcd(g, a);
    }
    cout << g << endl;
    return 0;
}
