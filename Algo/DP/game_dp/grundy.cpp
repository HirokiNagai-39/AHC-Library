// Grundy数 (Nim / ゲーム理論)
// Sprague-Grundy定理: 複数の独立なゲームのGrundy数のXORで勝敗判定
// 例題: ABC206 D (類題), Nim
// 計算量: O(状態数 * 遷移数)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Grundy数 ===
// moves[i] = 状態iから遷移可能な状態の集合
// grundy[i] = mex(grundy[遷移先])
// mex = minimum excludant (集合に含まれない最小の非負整数)
int mex(const set<int>& s){
    int m = 0;
    while(s.count(m)) m++;
    return m;
}

vector<int> calc_grundy(int n, const vector<vector<int>>& moves){
    vector<int> grundy(n, -1);
    function<int(int)> solve = [&](int state) -> int {
        if(grundy[state] != -1) return grundy[state];
        set<int> reachable;
        for(int nxt : moves[state]){
            reachable.insert(solve(nxt));
        }
        return grundy[state] = mex(reachable);
    };
    for(int i = 0; i < n; i++) solve(i);
    return grundy;
}

// === 例題: Nim ===
// N個の山, 各山にa[i]個の石. 交互に任意の山から1個以上取る. 取れなくなったら負け
// Grundy定理: XOR(a[i]) != 0 なら先手勝ち
int main(){
    int N;
    cin >> N;
    long long xor_sum = 0;
    for(int i = 0; i < N; i++){
        long long a; cin >> a;
        xor_sum ^= a;
    }
    cout << (xor_sum != 0 ? "First" : "Second") << endl;
    return 0;
}
