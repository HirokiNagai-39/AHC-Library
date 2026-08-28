// ローリングハッシュ (文字列ハッシュ)
// 部分文字列の一致判定を O(1) で行う
// 計算量: 前処理 O(N), クエリ O(1)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Rolling Hash ===
struct RollingHash {
    using ull = unsigned long long;
    static constexpr ull MOD = (1ULL << 61) - 1;
    static constexpr ull BASE = 131; // 衝突回避のため変更可

    vector<ull> hash_, pow_;

    static ull mod(ull a){
        a = (a >> 61) + (a & MOD);
        return a >= MOD ? a - MOD : a;
    }

    static ull mul(ull a, ull b){
        unsigned __int128 c = (unsigned __int128)a * b;
        return mod((ull)(c >> 61) + (ull)(c & MOD));
    }

    RollingHash(const string& s){
        int n = s.size();
        hash_.resize(n + 1);
        pow_.resize(n + 1);
        hash_[0] = 0;
        pow_[0] = 1;
        for(int i = 0; i < n; i++){
            hash_[i + 1] = mod(mul(hash_[i], BASE) + s[i]);
            pow_[i + 1] = mul(pow_[i], BASE);
        }
    }

    // s[l..r) のハッシュ値
    ull get(int l, int r){
        return mod(hash_[r] + MOD * 4 - mul(hash_[l], pow_[r - l]));
    }

    // s[l1..r1) と s[l2..r2) が一致するか
    bool match(int l1, int r1, int l2, int r2){
        return get(l1, r1) == get(l2, r2);
    }
};

// === 例題: 文字列中のパターン出現位置 ===
int main(){
    string S, T;
    cin >> S >> T;
    int n = S.size(), m = T.size();

    RollingHash hs(S), ht(T);
    ull target = ht.get(0, m);

    vector<int> positions;
    for(int i = 0; i + m <= n; i++){
        if(hs.get(i, i + m) == target){
            positions.push_back(i);
        }
    }

    cout << positions.size() << "\n";
    for(int p : positions) cout << p << "\n";
    return 0;
}
