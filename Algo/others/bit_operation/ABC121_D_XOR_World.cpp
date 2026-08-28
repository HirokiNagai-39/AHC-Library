// ABC121 D - XOR World
// https://atcoder.jp/contests/abc121/tasks/abc121_d
// f(A,B) = A xor (A+1) xor ... xor B。
// f(0,N)の規則性: N%4で場合分け。f(A,B)=f(0,B) xor f(0,A-1)。
// 計算量: O(1)
#include <bits/stdc++.h>
using namespace std;

// 0 xor 1 xor ... xor N
long long xor_upto(long long n) {
    if (n < 0) return 0;
    switch (n % 4) {
        case 0: return n;
        case 1: return 1;
        case 2: return n + 1;
        case 3: return 0;
    }
    return 0;
}

int main() {
    long long A, B;
    cin >> A >> B;
    cout << (xor_upto(B) ^ xor_upto(A - 1)) << endl;
    return 0;
}
