// Z-algorithm
// Z[i] = S[0..] と S[i..] の最長共通接頭辞の長さ
// 例題: ABC150 F (部分的に関連)
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Z-algorithm ===
vector<int> z_algorithm(const string& s){
    int n = s.size();
    vector<int> z(n, 0);
    z[0] = n;
    int i = 1, j = 0;
    while(i < n){
        while(i + j < n && s[j] == s[i + j]) j++;
        z[i] = j;
        if(j == 0){ i++; continue; }
        int k = 1;
        while(k < j && k + z[k] < j){
            z[i + k] = z[k];
            k++;
        }
        i += k;
        j -= k;
    }
    return z;
}

// === 例題: パターンマッチング (Z-algorithm版) ===
// T が S の中に何回現れるか
int main(){
    string S, T;
    cin >> T >> S;
    int n = S.size(), m = T.size();

    // T + "$" + S を結合して Z-algorithm
    string concat = T + "$" + S;
    auto z = z_algorithm(concat);

    int count = 0;
    for(int i = m + 1; i < (int)concat.size(); i++){
        if(z[i] >= m) count++;
    }
    cout << count << endl;
    return 0;
}
