// めぐる式二分探索テンプレート + 実数版
// ok/ng を明示的に管理。条件を満たす境界を求める。
// 計算量: 整数版 O(log(幅)), 実数版 O(反復回数)
#include <bits/stdc++.h>
using namespace std;

// === 整数二分探索 (めぐる式) ===
// f(ok)=true, f(ng)=false を前提に、境界のok側を返す
template<typename F>
long long meguru_bs(long long ng, long long ok, F f) {
    while (abs(ok - ng) > 1) {
        long long mid = ng + (ok - ng) / 2;
        (f(mid) ? ok : ng) = mid;
    }
    return ok;
}

// === 実数二分探索 ===
template<typename F>
double meguru_bs_real(double ng, double ok, F f, int iter = 200) {
    for (int i = 0; i < iter; i++) {
        double mid = (ng + ok) / 2;
        (f(mid) ? ok : ng) = mid;
    }
    return ok;
}

// === 例題: ソート済み配列で値v以上の最小インデックス ===
int main() {
    int N, Q;
    cin >> N >> Q;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;
    sort(A.begin(), A.end());

    while (Q--) {
        long long v;
        cin >> v;
        // A[idx] >= v となる最小idx
        long long idx = meguru_bs(-1LL, (long long)N, [&](long long mid) {
            return A[mid] >= v;
        });
        cout << idx << "\n";
    }
    return 0;
}
