// 複数パターンマッチング (ハッシュセット版)
// 複数のパターンの出現位置を一括検索。Aho-Corasickの簡易版。
// 全パターン同じ長さの場合に特に有効。
// 計算量: O((N+M*K)) N=テキスト長, M=パターン数, K=パターン長
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

int main() {
    string text;
    int M;
    cin >> text >> M;
    RollingHash rh(text);
    int n = text.size();

    // パターンを長さごとにグループ化
    map<int, set<unsigned long long>> patterns; // len -> set of hashes
    map<int, map<unsigned long long, string>> pat_str;
    for (int i = 0; i < M; i++) {
        string p;
        cin >> p;
        RollingHash ph(p);
        auto h = ph.get(0, p.size());
        patterns[p.size()].insert(h);
        pat_str[p.size()][h] = p;
    }

    // 各長さについてテキストをスキャン
    for (auto& [len, hset] : patterns) {
        for (int i = 0; i + len <= n; i++) {
            auto h = rh.get(i, i + len);
            if (hset.count(h))
                cout << "Found \"" << pat_str[len][h] << "\" at pos " << i << "\n";
        }
    }
    return 0;
}
