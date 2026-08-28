// ダブリング
// 関数の繰り返し適用を O(log K) で計算
// 例題: ABC167 D - Teleporter
// https://atcoder.jp/contests/abc167/tasks/abc167_d
// 計算量: 前処理 O(N log K), クエリ O(log K)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: ダブリング ===
struct Doubling {
    int n, LOG;
    vector<vector<int>> nxt;

    // n: 状態数, max_k: 最大ステップ数
    Doubling(int n, long long max_k) : n(n) {
        LOG = 1;
        while((1LL << LOG) <= max_k) LOG++;
        nxt.assign(LOG, vector<int>(n));
    }

    // 1ステップの遷移を設定
    void set_next(int v, int u){ nxt[0][v] = u; }

    // ダブリングテーブル構築
    void build(){
        for(int k = 0; k + 1 < LOG; k++){
            for(int v = 0; v < n; v++){
                nxt[k+1][v] = nxt[k][nxt[k][v]];
            }
        }
    }

    // vからK回遷移した先
    int query(int v, long long K){
        for(int k = 0; k < LOG; k++){
            if((K >> k) & 1) v = nxt[k][v];
        }
        return v;
    }
};

// === 例題: ABC167 D ===
int main(){
    int N;
    long long K;
    cin >> N >> K;

    Doubling db(N, K);
    for(int i = 0; i < N; i++){
        int a; cin >> a; a--;
        db.set_next(i, a);
    }
    db.build();

    cout << db.query(0, K) + 1 << endl;
    return 0;
}
