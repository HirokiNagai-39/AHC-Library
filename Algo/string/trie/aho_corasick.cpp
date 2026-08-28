// Aho-Corasick (エイホ-コラシック法)
// 複数パターンの同時マッチングを行うオートマトン
// 計算量: 構築 O(Σ|P_i| * σ), 検索 O(|T| + (マッチ数))
//   σ: アルファベットサイズ
// 例題: ABC268 (応用)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Aho-Corasick ===
struct AhoCorasick {
    static constexpr int SIGMA = 26;

    struct Node {
        int children[SIGMA];
        int fail;       // failure link
        int output;     // output link (最も近い辞書上の祖先)
        int pattern_id; // この頂点で終わるパターン (-1 ならなし)
        int count;      // この頂点から output link を辿って到達できるパターン数
        Node() : fail(0), output(-1), pattern_id(-1), count(0) {
            fill(children, children + SIGMA, -1);
        }
    };

    vector<Node> nodes;

    AhoCorasick() { nodes.emplace_back(); }

    // パターンを追加 (id = パターン番号)
    void add_pattern(const string& s, int id) {
        int cur = 0;
        for (char c : s) {
            int idx = c - 'a';
            if (nodes[cur].children[idx] == -1) {
                nodes[cur].children[idx] = nodes.size();
                nodes.emplace_back();
            }
            cur = nodes[cur].children[idx];
        }
        nodes[cur].pattern_id = id;
    }

    // failure link と output link を構築
    void build() {
        queue<int> que;
        // root の子を初期化
        for (int c = 0; c < SIGMA; c++) {
            if (nodes[0].children[c] == -1) {
                nodes[0].children[c] = 0; // root に戻す
            } else {
                nodes[nodes[0].children[c]].fail = 0;
                que.push(nodes[0].children[c]);
            }
        }
        // BFS で failure link を構築
        while (!que.empty()) {
            int v = que.front(); que.pop();
            // output link
            if (nodes[nodes[v].fail].pattern_id != -1) {
                nodes[v].output = nodes[v].fail;
            } else {
                nodes[v].output = nodes[nodes[v].fail].output;
            }
            // count
            nodes[v].count = (nodes[v].pattern_id != -1 ? 1 : 0)
                           + (nodes[v].output != -1 ? nodes[nodes[v].output].count : 0);

            for (int c = 0; c < SIGMA; c++) {
                if (nodes[v].children[c] == -1) {
                    // goto 関数を failure link 経由で埋める
                    nodes[v].children[c] = nodes[nodes[v].fail].children[c];
                } else {
                    nodes[nodes[v].children[c]].fail = nodes[nodes[v].fail].children[c];
                    que.push(nodes[v].children[c]);
                }
            }
        }
    }

    // テキスト T 中の各パターンの出現回数を返す
    // result[id] = パターン id の出現回数
    vector<int> search(const string& t, int num_patterns) {
        vector<int> result(num_patterns, 0);
        int cur = 0;
        for (char c : t) {
            cur = nodes[cur].children[c - 'a'];
            // output link を辿って全マッチを列挙
            int tmp = cur;
            while (tmp != -1 && tmp != 0) {
                if (nodes[tmp].pattern_id != -1) {
                    result[nodes[tmp].pattern_id]++;
                }
                tmp = nodes[tmp].output;
            }
        }
        return result;
    }

    // テキスト T 中にマッチするパターンの総出現回数
    long long count_all_matches(const string& t) {
        long long total = 0;
        int cur = 0;
        for (char c : t) {
            cur = nodes[cur].children[c - 'a'];
            total += nodes[cur].count;
        }
        return total;
    }
};

// === 例題: テキスト中の各パターンの出現回数 ===
int main() {
    int n;
    cin >> n;
    AhoCorasick aho;
    vector<string> patterns(n);
    for (int i = 0; i < n; i++) {
        cin >> patterns[i];
        aho.add_pattern(patterns[i], i);
    }
    aho.build();

    string t;
    cin >> t;
    auto result = aho.search(t, n);

    for (int i = 0; i < n; i++) {
        cout << patterns[i] << ": " << result[i] << "\n";
    }
    return 0;
}
