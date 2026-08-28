// 座標圧縮 + BIT (転倒数)
// 配列の転倒数を座圧+BITで O(N log N) で計算
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

struct BIT {
    int n;
    vector<int> tree;
    BIT(int n) : n(n), tree(n + 1, 0) {}
    void add(int i, int v) { for (; i <= n; i += i & -i) tree[i] += v; }
    int sum(int i) { int s = 0; for (; i > 0; i -= i & -i) s += tree[i]; return s; }
};

int main() {
    int N;
    cin >> N;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    // 座標圧縮
    vector<long long> sorted_a = A;
    sort(sorted_a.begin(), sorted_a.end());
    sorted_a.erase(unique(sorted_a.begin(), sorted_a.end()), sorted_a.end());
    vector<int> comp(N);
    for (int i = 0; i < N; i++)
        comp[i] = lower_bound(sorted_a.begin(), sorted_a.end(), A[i]) - sorted_a.begin() + 1;

    // BITで転倒数を計算
    int sz = sorted_a.size();
    BIT bit(sz);
    long long inv = 0;
    for (int i = 0; i < N; i++) {
        inv += i - bit.sum(comp[i]);
        bit.add(comp[i], 1);
    }
    cout << inv << endl;
    return 0;
}
