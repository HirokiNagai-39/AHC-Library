// Mo's Algorithm (平方分割によるオフラインクエリ処理)
// 区間クエリ [l, r) をオフラインで効率的に処理する
// 追加・削除が O(α) のとき全体 O((N + Q) √N · α)
// 計算量: O((N + Q) √N)
// 例題: ABC174 F, ABC293 G
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Mo's Algorithm ===
struct Mo {
    int n, q, block;
    vector<int> ql, qr, order;

    Mo(int n, int q) : n(n), q(q), ql(q), qr(q), order(q) {
        block = max(1, (int)sqrt(n));
        iota(order.begin(), order.end(), 0);
    }

    // クエリ追加: [l, r)
    void add_query(int i, int l, int r) {
        ql[i] = l;
        qr[i] = r;
    }

    // クエリをソートして処理
    // add_left(idx), add_right(idx): 要素を追加
    // del_left(idx), del_right(idx): 要素を削除
    // answer(query_id): クエリに回答
    template<class AL, class AR, class DL, class DR, class ANS>
    void run(AL add_left, AR add_right, DL del_left, DR del_right, ANS answer) {
        sort(order.begin(), order.end(), [&](int a, int b) {
            int ab = ql[a] / block, bb = ql[b] / block;
            if (ab != bb) return ab < bb;
            return (ab & 1) ? qr[a] > qr[b] : qr[a] < qr[b];
        });
        int cur_l = 0, cur_r = 0;
        for (int qi : order) {
            while (cur_r < qr[qi]) add_right(cur_r++);
            while (cur_l > ql[qi]) add_left(--cur_l);
            while (cur_r > qr[qi]) del_right(--cur_r);
            while (cur_l < ql[qi]) del_left(cur_l++);
            answer(qi);
        }
    }
};

// === 例題: 区間内の異なる値の個数 ===
// ABC174 F - Range Set Query
// N 個の色 c[i] からなる列に対し、Q 個のクエリ [l, r] の色の種類数を答える
int main() {
    int n, q;
    cin >> n >> q;
    vector<int> c(n);
    for (int i = 0; i < n; i++) cin >> c[i];

    Mo mo(n, q);
    for (int i = 0; i < q; i++) {
        int l, r;
        cin >> l >> r;
        mo.add_query(i, l - 1, r); // 1-indexed → [l-1, r)
    }

    vector<int> cnt(500001, 0);
    int kinds = 0;

    auto add = [&](int idx) {
        if (cnt[c[idx]]++ == 0) kinds++;
    };
    auto del = [&](int idx) {
        if (--cnt[c[idx]] == 0) kinds--;
    };

    vector<int> ans(q);
    mo.run(add, add, del, del, [&](int qi) { ans[qi] = kinds; });

    for (int i = 0; i < q; i++) cout << ans[i] << "\n";
    return 0;
}
