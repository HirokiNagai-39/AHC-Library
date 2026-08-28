// Manacher's Algorithm (マナカー法)
// 各位置を中心とする最長回文半径を O(N) で求める
// 計算量: O(N)
// 例題: ABC307 (応用), 典型90 #64
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Manacher ===
// radius[i] = s[i] を中心とする最長回文の半径 (s[i-r+1..i+r-1] が回文)
// 奇数長の回文のみ扱う
vector<int> manacher(const string& s) {
    int n = s.size();
    vector<int> radius(n);
    int center = 0, right = 0;
    for (int i = 0; i < n; i++) {
        int r = (right > i) ? min(radius[2 * center - i], right - i) : 1;
        while (i - r >= 0 && i + r < n && s[i - r] == s[i + r]) r++;
        radius[i] = r;
        if (i + r > right) {
            center = i;
            right = i + r;
        }
    }
    return radius;
}

// === ライブラリ: Manacher (偶数長も扱う版) ===
// ダミー文字を挿入して偶数長・奇数長の両方を統一的に扱う
// 例: "abba" -> "$a$b$b$a$" として Manacher を適用
// 元の文字列上での回文長 = radius[i] - 1
vector<int> manacher_even_odd(const string& s) {
    // ダミー文字を挿入
    string t = "$";
    for (char c : s) {
        t += c;
        t += '$';
    }
    return manacher(t);
}

// === 例題: 全ての回文部分文字列を列挙 ===
int main() {
    string s;
    cin >> s;
    int n = s.size();

    // 偶数長・奇数長の両方を扱う
    auto rad = manacher_even_odd(s);
    // rad は "$a$b$..." 上の回文半径
    // t[i] がダミー文字('$')の位置: 偶数長回文の中心
    // t[i] が元の文字の位置: 奇数長回文の中心

    string t = "$";
    for (char c : s) { t += c; t += '$'; }

    // 最長回文部分文字列
    int best_center = 0, best_len = 0;
    for (int i = 0; i < (int)t.size(); i++) {
        int len = rad[i] - 1; // 元の文字列上での回文長
        if (len > best_len) {
            best_len = len;
            best_center = i;
        }
    }
    // t 上の位置 [best_center - rad[best_center] + 1, best_center + rad[best_center] - 1]
    // 元の文字列上の開始位置
    int start = (best_center - (rad[best_center] - 1)) / 2;
    cout << s.substr(start, best_len) << "\n";

    return 0;
}
