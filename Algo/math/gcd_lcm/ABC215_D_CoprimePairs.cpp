// ABC215 D - Coprime 2
// https://atcoder.jp/contests/abc215/tasks/abc215_d
// A中のどの要素とも互いに素な 1~M を列挙。
// Aの全要素の素因数を集め、篩でそれらを因数に持つ数を除外。
// 計算量: O(N sqrt(max A) + M log log M)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M;
    cin >> N >> M;
    vector<int> A(N);
    for (auto& a : A) cin >> a;

    // Aに現れる素因数を収集
    set<int> primes;
    for (auto a : A) {
        for (int p = 2; p * p <= a; p++) {
            if (a % p == 0) {
                primes.insert(p);
                while (a % p == 0) a /= p;
            }
        }
        if (a > 1) primes.insert(a);
    }

    // 篩: primeの倍数を除外
    vector<bool> ok(M + 1, true);
    for (int p : primes)
        for (int j = p; j <= M; j += p)
            ok[j] = false;

    vector<int> ans;
    for (int i = 1; i <= M; i++)
        if (ok[i]) ans.push_back(i);

    cout << ans.size() << "\n";
    for (int x : ans) cout << x << "\n";
    return 0;
}
