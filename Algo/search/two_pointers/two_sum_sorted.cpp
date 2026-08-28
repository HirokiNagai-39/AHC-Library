// Two Sum (ソート済み配列版)
// ソート済み配列から和がtargetとなるペアを探す。
// 左右からポインタを狭める。
// 計算量: O(N log N) (ソート含む)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long target;
    cin >> N >> target;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;
    sort(A.begin(), A.end());

    int l = 0, r = N - 1;
    long long count = 0;
    while (l < r) {
        long long s = A[l] + A[r];
        if (s == target) {
            if (A[l] == A[r]) {
                long long len = r - l + 1;
                count += len * (len - 1) / 2;
                break;
            }
            long long cl = 1, cr = 1;
            while (l + 1 < r && A[l] == A[l+1]) { l++; cl++; }
            while (r - 1 > l && A[r] == A[r-1]) { r--; cr++; }
            count += cl * cr;
            l++; r--;
        } else if (s < target) {
            l++;
        } else {
            r--;
        }
    }
    cout << count << endl;
    return 0;
}
