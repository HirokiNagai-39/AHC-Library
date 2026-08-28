// ABC076 C - Dubious Document 2
// https://atcoder.jp/contests/abc076/tasks/abc076_c
// S中の'?'を置き換えてTを含む辞書順最小文字列を作る。
// 貪欲に後ろからTをマッチさせ、残りの?をaに。
// 計算量: O(|S| * |T|)
#include <bits/stdc++.h>
using namespace std;

int main() {
    string S, T;
    cin >> S >> T;
    int n = S.size(), m = T.size();

    // Tを配置できる最も後ろの位置を探す(辞書順最小のため後ろ優先)
    int pos = -1;
    for (int i = n - m; i >= 0; i--) {
        bool ok = true;
        for (int j = 0; j < m; j++) {
            if (S[i+j] != '?' && S[i+j] != T[j]) { ok = false; break; }
        }
        if (ok) { pos = i; break; }
    }

    if (pos == -1) {
        cout << "UNRESTORABLE" << endl;
        return 0;
    }

    for (int j = 0; j < m; j++) S[pos+j] = T[j];
    for (auto& c : S) if (c == '?') c = 'a';
    cout << S << endl;
    return 0;
}
