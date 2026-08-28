// 回文判定をハッシュで行う
// 正方向と逆方向のハッシュを比較。部分文字列が回文か O(1) 判定。
// 計算量: O(N) 前処理, O(1) クエリ
#include <bits/stdc++.h>
using namespace std;

struct RollingHash {
    using ull = unsigned long long;
    static constexpr ull MOD = (1ULL << 61) - 1, BASE = 131;
    vector<ull> h, pw;
    static ull mod(ull a) { a = (a >> 61) + (a & MOD); return a >= MOD ? a - MOD : a; }
    static ull mul(ull a, ull b) { unsigned __int128 c = (unsigned __int128)a*b; return mod((ull)(c>>61)+(ull)(c&MOD)); }
    RollingHash() {}
    RollingHash(const string& s) : h(s.size()+1), pw(s.size()+1) {
        h[0]=0; pw[0]=1;
        for (int i=0;i<(int)s.size();i++) { h[i+1]=mod(mul(h[i],BASE)+s[i]); pw[i+1]=mul(pw[i],BASE); }
    }
    ull get(int l, int r) { return mod(h[r]+MOD*4-mul(h[l],pw[r-l])); }
};

int main() {
    string S;
    cin >> S;
    int n = S.size();
    string R(S.rbegin(), S.rend());
    RollingHash fwd(S), rev(R);

    // S[l..r) が回文か判定: fwd.get(l,r) == rev.get(n-r, n-l)
    auto is_palindrome = [&](int l, int r) -> bool {
        return fwd.get(l, r) == rev.get(n - r, n - l);
    };

    int Q;
    cin >> Q;
    while (Q--) {
        int l, r;
        cin >> l >> r;
        cout << (is_palindrome(l, r) ? "Yes" : "No") << "\n";
    }
    return 0;
}
