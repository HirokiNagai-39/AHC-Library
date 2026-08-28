// popcount応用: ビットごとに寄与を数える典型
// 例: 全ペア(i,j)のA[i] AND A[j]の和
// 各ビットkについて、そのビットが立っている要素数cntとすると
// 寄与 = 2^k * C(cnt, 2)
// 計算量: O(N * 60)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    // 全ペアの AND の和
    long long ans_and = 0;
    for (int k = 0; k < 60; k++) {
        long long cnt = 0;
        for (int i = 0; i < N; i++)
            if (A[i] >> k & 1) cnt++;
        ans_and += (1LL << k) * cnt * (cnt - 1) / 2;
    }
    cout << "Sum of AND: " << ans_and << "\n";

    // 全ペアの OR の和
    long long ans_or = 0;
    for (int k = 0; k < 60; k++) {
        long long cnt = 0;
        for (int i = 0; i < N; i++)
            if (A[i] >> k & 1) cnt++;
        // OR のビットkが立つペア数 = 全ペア - 両方0のペア
        long long zero = N - cnt;
        ans_or += (1LL << k) * ((long long)N*(N-1)/2 - zero*(zero-1)/2);
    }
    cout << "Sum of OR: " << ans_or << "\n";

    // 全ペアの XOR の和
    long long ans_xor = 0;
    for (int k = 0; k < 60; k++) {
        long long cnt = 0;
        for (int i = 0; i < N; i++)
            if (A[i] >> k & 1) cnt++;
        ans_xor += (1LL << k) * cnt * (N - cnt);
    }
    cout << "Sum of XOR: " << ans_xor << "\n";
    return 0;
}
