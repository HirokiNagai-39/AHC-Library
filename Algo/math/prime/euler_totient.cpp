// オイラーのトーシェント関数 phi(N)
// 1~Nでnと互いに素な数の個数
// phi(N) = N * prod(1 - 1/p) for each prime factor p of N
// 計算量: 単一 O(sqrt(N)), 篩 O(N log log N)
#include <bits/stdc++.h>
using namespace std;

// 単一のphi(n)
long long euler_phi(long long n) {
    long long res = n;
    for (long long p = 2; p * p <= n; p++) {
        if (n % p == 0) {
            while (n % p == 0) n /= p;
            res -= res / p;
        }
    }
    if (n > 1) res -= res / n;
    return res;
}

// 篩でphi(1)~phi(N)を一括計算
vector<int> euler_phi_table(int n) {
    vector<int> phi(n + 1);
    iota(phi.begin(), phi.end(), 0);
    for (int i = 2; i <= n; i++) {
        if (phi[i] == i) { // iは素数
            for (int j = i; j <= n; j += i)
                phi[j] -= phi[j] / i;
        }
    }
    return phi;
}

int main() {
    int N;
    cin >> N;
    // 単一クエリ
    cout << euler_phi(N) << "\n";
    // テーブル版
    auto phi = euler_phi_table(N);
    long long sum = 0;
    for (int i = 1; i <= N; i++) sum += phi[i];
    cout << sum << "\n";
    return 0;
}
