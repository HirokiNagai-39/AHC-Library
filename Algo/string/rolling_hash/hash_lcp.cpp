// ハッシュによるLCP (最長共通接頭辞)
// 2つの文字列(or 部分文字列)のLCPを二分探索+ハッシュで O(log N)
// 計算量: O(N) 前処理, O(log N) クエリ
#include <bits/stdc++.h>
using namespace std;

struct RollingHash {
    using ull = unsigned long long;
    static constexpr ull MOD = (1ULL << 61) - 1, BASE = 131;
    vector<ull> h, pw;
    static ull mod(ull a) { a = (a >> 61) + (a & MOD); return a >= MOD ? a - MOD : a; }
    static ull mul(ull a, ull b) { unsigned __int128 c = (unsigned __int128)a*b; return mod((ull)(c>>61)+(ull)(c&MOD)); }
    RollingHash(const string& s) : h(s.size()+1), pw(s.size()+1) {
        h[0]=0; pw[0]=1;
        for (int i=0;i<(int)s.size();i++) { h[i+1]=mod(mul(h[i],BASE)+s[i]); pw[i+1]=mul(pw[i],BASE); }
    }
    ull get(int l, int r) { return mod(h[r]+MOD*4-mul(h[l],pw[r-l])); }
};

// S[a..]とS[b..]のLCP長
int lcp(RollingHash& rh, int a, int b, int n) {
    int lo = 0, hi = min(n - a, n - b);
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        if (rh.get(a, a+mid) == rh.get(b, b+mid)) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}

int main() {
    string S;
    cin >> S;
    int n = S.size();
    RollingHash rh(S);

    int Q;
    cin >> Q;
    while (Q--) {
        int a, b;
        cin >> a >> b;
        cout << lcp(rh, a, b, n) << "\n";
    }
    return 0;
}
