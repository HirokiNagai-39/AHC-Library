// XOR基底 (線形基底 / 掃き出し法)
// N個の数のXORで作れる値の線形空間。基底のサイズがrank。
// 作れる値の数 = 2^rank。最大XOR値も求められる。
// 計算量: O(N * 60)
#include <bits/stdc++.h>
using namespace std;

struct XorBasis {
    static constexpr int BITS = 60;
    long long basis[BITS] = {};
    int rank_ = 0;

    // 値vを基底に追加。追加できたらtrue。
    bool add(long long v) {
        for (int i = BITS - 1; i >= 0; i--) {
            if (!(v >> i & 1)) continue;
            if (!basis[i]) {
                basis[i] = v;
                rank_++;
                return true;
            }
            v ^= basis[i];
        }
        return false; // 既存の基底で表現可能
    }

    // 基底のXORで作れる最大値
    long long max_xor() {
        long long res = 0;
        for (int i = BITS - 1; i >= 0; i--)
            res = max(res, res ^ basis[i]);
        return res;
    }

    // vがこの基底で表現可能か
    bool contains(long long v) {
        for (int i = BITS - 1; i >= 0; i--) {
            if (!(v >> i & 1)) continue;
            if (!basis[i]) return false;
            v ^= basis[i];
        }
        return true;
    }
};

int main() {
    int N;
    cin >> N;
    XorBasis xb;
    for (int i = 0; i < N; i++) {
        long long v;
        cin >> v;
        xb.add(v);
    }
    cout << "Rank: " << xb.rank_ << "\n";
    cout << "Max XOR: " << xb.max_xor() << "\n";
    cout << "Distinct values: " << (1LL << xb.rank_) << "\n";
    return 0;
}
