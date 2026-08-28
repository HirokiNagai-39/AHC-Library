// ABC141 E - Who Says a Pun?
// https://atcoder.jp/contests/abc141/tasks/abc141_e
// 重複しない最長共通部分文字列長。二分探索+ローリングハッシュ。
// 計算量: O(N log N)
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
    ull get(int l,int r) { return mod(h[r]+MOD*4-mul(h[l],pw[r-l])); }
};

int main() {
    int N;
    string S;
    cin >> N >> S;
    RollingHash rh(S);

    auto check = [&](int len) -> bool {
        // 長さlenの部分文字列で、重複しない(開始位置が離れた)ペアがあるか
        map<unsigned long long, int> first_pos;
        for (int i = 0; i + len <= N; i++) {
            auto h = rh.get(i, i + len);
            if (first_pos.count(h)) {
                if (i >= first_pos[h] + len) return true;
            } else {
                first_pos[h] = i;
            }
        }
        return false;
    };

    int lo = 0, hi = N / 2;
    while (hi - lo > 0) {
        int mid = (lo + hi + 1) / 2;
        if (check(mid)) lo = mid;
        else hi = mid - 1;
    }
    cout << lo << endl;
    return 0;
}
