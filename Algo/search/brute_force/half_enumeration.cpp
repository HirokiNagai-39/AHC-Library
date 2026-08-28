// 半分全列挙 (Meet in the Middle)
// N個の要素を前半・後半に分け、各2^(N/2)通りを列挙して結合。
// 例: N個の整数から和がTになる部分集合の存在判定。
// 計算量: O(2^(N/2) * log(2^(N/2)))
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long T;
    cin >> N >> T;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    int h1 = N / 2, h2 = N - h1;

    // 前半の全部分集合の和を列挙
    vector<long long> sums1;
    for (int bit = 0; bit < (1 << h1); bit++) {
        long long s = 0;
        for (int i = 0; i < h1; i++)
            if (bit >> i & 1) s += A[i];
        sums1.push_back(s);
    }
    sort(sums1.begin(), sums1.end());

    // 後半の各部分集合和に対し、前半で T-s を二分探索
    bool found = false;
    for (int bit = 0; bit < (1 << h2); bit++) {
        long long s = 0;
        for (int i = 0; i < h2; i++)
            if (bit >> i & 1) s += A[h1 + i];
        if (binary_search(sums1.begin(), sums1.end(), T - s)) {
            found = true;
            break;
        }
    }
    cout << (found ? "Yes" : "No") << endl;
    return 0;
}
