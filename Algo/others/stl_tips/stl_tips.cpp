// C++ STL 頻出テクニック集
// 競プロで使う主要なSTL機能のチートシート
#include <bits/stdc++.h>
using namespace std;

void demo(){
    // ========== ソート ==========
    vector<int> v = {3, 1, 4, 1, 5};
    sort(v.begin(), v.end());                    // 昇順
    sort(v.begin(), v.end(), greater<int>());    // 降順
    // カスタムソート
    sort(v.begin(), v.end(), [](int a, int b){ return a > b; });

    // ========== 二分探索 (ソート済み配列) ==========
    // lower_bound: x以上の最初の位置
    // upper_bound: xより大きい最初の位置
    auto it = lower_bound(v.begin(), v.end(), 3);
    int idx = it - v.begin(); // インデックス
    // x以上の要素数: v.end() - lower_bound(v.begin(), v.end(), x)
    // xの個数: upper_bound - lower_bound

    // ========== set / multiset ==========
    set<int> s = {1, 2, 3};
    s.insert(4);
    s.erase(2);
    s.count(3);             // 0 or 1
    s.find(3);              // イテレータ (なければ end())
    auto lo = s.lower_bound(2); // 2以上の最小要素

    multiset<int> ms = {1, 1, 2, 3};
    ms.erase(ms.find(1));   // 1つだけ消す (erase(1)は全部消える)

    // ========== map ==========
    map<string, int> mp;
    mp["apple"] = 3;
    mp["banana"]++;         // なければ0で初期化されて+1
    for(auto& [key, val] : mp){
        // key, val を使える
    }

    // ========== priority_queue ==========
    priority_queue<int> pq;                           // 最大値が先
    priority_queue<int, vector<int>, greater<int>> minpq; // 最小値が先
    pq.push(3); pq.push(1); pq.push(4);
    pq.top(); // 4
    pq.pop();

    // ========== deque ==========
    deque<int> dq;
    dq.push_front(1);
    dq.push_back(2);
    dq.pop_front();
    dq.pop_back();

    // ========== 文字列操作 ==========
    string str = "hello";
    str.substr(1, 3);       // "ell" (pos, len)
    str.find("ll");         // 2 (見つからなければ string::npos)
    reverse(str.begin(), str.end()); // "olleh"
    to_string(123);         // "123"
    stoi("456");            // 456
    stoll("123456789012");  // long long

    // ========== pair / tuple ==========
    pair<int, int> p = {1, 2};
    auto [a, b] = p; // C++17 構造化束縛

    // ========== accumulate (合計) ==========
    long long sum = accumulate(v.begin(), v.end(), 0LL);

    // ========== unique (重複除去, ソート済み前提) ==========
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());

    // ========== max_element / min_element ==========
    int mx = *max_element(v.begin(), v.end());
    int mn = *min_element(v.begin(), v.end());

    // ========== bit操作 ==========
    __builtin_popcount(15);      // 4 (ビットの1の数, int)
    __builtin_popcountll(15LL);  // 4 (long long版)
    __builtin_clz(8);            // 28 (先頭0の数)
    __builtin_ctz(8);            // 3 (末尾0の数)

    // ========== INF の定義 ==========
    // int:  static constexpr int INF = 1e9;
    // long long: static constexpr long long INF = 1e18;
}

int main(){
    // === 入出力高速化 (必須) ===
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // === よくある入力パターン ===
    int N; cin >> N;
    vector<int> a(N);
    for(int i = 0; i < N; i++) cin >> a[i];
    // or: for(auto& x : a) cin >> x;

    // === 出力 ===
    for(int i = 0; i < N; i++){
        cout << a[i];
        if(i < N-1) cout << " ";
    }
    cout << "\n"; // endlよりも高速

    return 0;
}
