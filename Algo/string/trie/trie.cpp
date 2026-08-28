// Trie (トライ木)
// 文字列の集合を木構造で管理する
// 計算量: 挿入 O(|S|), 検索 O(|S|)
// 例題: ABC287 E
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Trie ===
struct Trie {
    struct Node {
        int children[26];
        int count;  // この頂点を通る文字列の数
        bool is_end; // ここで終わる文字列があるか
        Node() : count(0), is_end(false) {
            fill(children, children + 26, -1);
        }
    };

    vector<Node> nodes;

    Trie() { nodes.emplace_back(); }

    void insert(const string& s) {
        int cur = 0;
        for (char c : s) {
            int idx = c - 'a';
            if (nodes[cur].children[idx] == -1) {
                nodes[cur].children[idx] = nodes.size();
                nodes.emplace_back();
            }
            cur = nodes[cur].children[idx];
            nodes[cur].count++;
        }
        nodes[cur].is_end = true;
    }

    // s がトライに存在するか
    bool search(const string& s) {
        int cur = 0;
        for (char c : s) {
            int idx = c - 'a';
            if (nodes[cur].children[idx] == -1) return false;
            cur = nodes[cur].children[idx];
        }
        return nodes[cur].is_end;
    }

    // s を接頭辞に持つ文字列の数
    int count_prefix(const string& s) {
        int cur = 0;
        for (char c : s) {
            int idx = c - 'a';
            if (nodes[cur].children[idx] == -1) return 0;
            cur = nodes[cur].children[idx];
        }
        return nodes[cur].count;
    }

    // s と登録済み文字列の最長共通接頭辞の長さ
    int longest_common_prefix(const string& s) {
        int cur = 0, len = 0;
        for (char c : s) {
            int idx = c - 'a';
            if (nodes[cur].children[idx] == -1) break;
            cur = nodes[cur].children[idx];
            len++;
        }
        return len;
    }
};

// === 例題: 各文字列と他の文字列の最長共通接頭辞 ===
int main() {
    int n;
    cin >> n;
    vector<string> s(n);
    Trie trie;
    for (int i = 0; i < n; i++) {
        cin >> s[i];
        trie.insert(s[i]);
    }
    for (int i = 0; i < n; i++) {
        // s[i] の各接頭辞について、他に同じ接頭辞を持つ文字列があるか
        int cur = 0, ans = 0;
        for (int j = 0; j < (int)s[i].size(); j++) {
            int idx = s[i][j] - 'a';
            cur = trie.nodes[cur].children[idx];
            if (trie.nodes[cur].count >= 2) ans = j + 1;
            else break;
        }
        cout << ans << "\n";
    }
    return 0;
}
