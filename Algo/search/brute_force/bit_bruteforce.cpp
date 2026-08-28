// ビット全探索
// N個の要素から部分集合を全列挙する
// 例題: ABC128 C - Switches
// https://atcoder.jp/contests/abc128/tasks/abc128_c
// 計算量: O(2^N * N)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N, M;
    cin >> N >> M;
    vector<vector<int>> s(M);
    vector<int> p(M);
    for(int j = 0; j < M; j++){
        int k; cin >> k;
        s[j].resize(k);
        for(int i = 0; i < k; i++){
            cin >> s[j][i];
            s[j][i]--; // 0-indexed
        }
    }
    for(int j = 0; j < M; j++) cin >> p[j];

    int ans = 0;
    // 2^N 通りの部分集合を全列挙
    for(int bit = 0; bit < (1 << N); bit++){
        bool ok = true;
        for(int j = 0; j < M; j++){
            int cnt = 0;
            for(int i : s[j]){
                if(bit >> i & 1) cnt++;
            }
            if(cnt % 2 != p[j]) ok = false;
        }
        if(ok) ans++;
    }
    cout << ans << endl;
    return 0;
}
