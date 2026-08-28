// ABC079 C - Train Ticket
// https://atcoder.jp/contests/abc079/tasks/abc079_c
// 4桁ABCDの間に+/-を入れて結果が7になる式を求める。
// 3箇所にビット全探索(2^3=8通り)。
// 計算量: O(1)
#include <bits/stdc++.h>
using namespace std;

int main() {
    string S;
    cin >> S;
    int d[4];
    for (int i = 0; i < 4; i++) d[i] = S[i] - '0';

    for (int bit = 0; bit < 8; bit++) {
        int val = d[0];
        for (int i = 0; i < 3; i++) {
            if (bit >> i & 1) val += d[i+1];
            else val -= d[i+1];
        }
        if (val == 7) {
            cout << d[0];
            for (int i = 0; i < 3; i++)
                cout << (bit >> i & 1 ? "+" : "-") << d[i+1];
            cout << "=7" << endl;
            return 0;
        }
    }
    return 0;
}
