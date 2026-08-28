// KMP法 (Knuth-Morris-Pratt)
// failure function (prefix function) + パターンマッチング
// 計算量: O(N + M)
#include <bits/stdc++.h>
using namespace std;

// prefix function: pi[i] = S[0..i] の接頭辞と接尾辞が一致する最大長
vector<int> prefix_function(const string& s) {
    int n = s.size();
    vector<int> pi(n, 0);
    for (int i = 1; i < n; i++) {
        int j = pi[i-1];
        while (j > 0 && s[i] != s[j]) j = pi[j-1];
        if (s[i] == s[j]) j++;
        pi[i] = j;
    }
    return pi;
}

// KMPマッチング: T中のPの出現位置を返す
vector<int> kmp_search(const string& T, const string& P) {
    auto pi = prefix_function(P);
    vector<int> res;
    int j = 0;
    for (int i = 0; i < (int)T.size(); i++) {
        while (j > 0 && T[i] != P[j]) j = pi[j-1];
        if (T[i] == P[j]) j++;
        if (j == (int)P.size()) {
            res.push_back(i - j + 1);
            j = pi[j-1];
        }
    }
    return res;
}

int main() {
    string T, P;
    cin >> T >> P;
    auto positions = kmp_search(T, P);
    cout << positions.size() << "\n";
    for (int p : positions) cout << p << "\n";
    return 0;
}
