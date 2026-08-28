// Z-algorithmでパターンマッチング (全出現位置)
// P + "$" + T を連結しZ配列を計算。Z[i]=len(P)ならマッチ。
// 計算量: O(N + M)
#include <bits/stdc++.h>
using namespace std;

vector<int> z_function(const string& s) {
    int n = s.size();
    vector<int> z(n, 0);
    int l = 0, r = 0;
    for (int i = 1; i < n; i++) {
        if (i < r) z[i] = min(r - i, z[i - l]);
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) z[i]++;
        if (i + z[i] > r) { l = i; r = i + z[i]; }
    }
    return z;
}

int main() {
    string T, P;
    cin >> T >> P;
    string S = P + "$" + T;
    auto z = z_function(S);
    int m = P.size();
    vector<int> positions;
    for (int i = m + 1; i < (int)S.size(); i++)
        if (z[i] == m) positions.push_back(i - m - 1);
    cout << positions.size() << "\n";
    for (int p : positions) cout << p << "\n";
    return 0;
}
