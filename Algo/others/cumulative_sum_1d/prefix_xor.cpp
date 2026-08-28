// XOR累積和
// 区間XOR A[l] xor A[l+1] xor ... xor A[r] を O(1) で計算。
// pxor[i] = A[0] xor ... xor A[i-1]
// 区間[l,r] = pxor[r+1] xor pxor[l]
// 計算量: O(N) 前処理, O(1) クエリ
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, Q;
    cin >> N;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    vector<long long> pxor(N + 1, 0);
    for (int i = 0; i < N; i++) pxor[i+1] = pxor[i] ^ A[i];

    cin >> Q;
    while (Q--) {
        int l, r;
        cin >> l >> r; // 0-indexed [l, r]
        cout << (pxor[r+1] ^ pxor[l]) << "\n";
    }
    return 0;
}
