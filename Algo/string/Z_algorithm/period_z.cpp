// 文字列の最小周期をZ配列で求める
// Z[i] + i >= N かつ N % i == 0 なら i が周期
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

vector<int> z_function(const string& s) {
    int n = s.size();
    vector<int> z(n, 0);
    z[0] = n;
    int l = 0, r = 0;
    for (int i = 1; i < n; i++) {
        if (i < r) z[i] = min(r - i, z[i - l]);
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) z[i]++;
        if (i + z[i] > r) { l = i; r = i + z[i]; }
    }
    return z;
}

int main() {
    string S;
    cin >> S;
    int n = S.size();
    auto z = z_function(S);

    int min_period = n;
    for (int i = 1; i < n; i++) {
        if (z[i] + i >= n && n % i == 0) {
            min_period = i;
            break;
        }
    }
    cout << min_period << endl;
    return 0;
}
