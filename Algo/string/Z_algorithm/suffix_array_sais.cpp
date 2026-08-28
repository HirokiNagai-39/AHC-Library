// Suffix Array (SA-IS algorithm)
// 接尾辞配列を O(N) で構築する線形時間アルゴリズム
// 計算量: O(N) 構築
// 既存の suffix_array.cpp は O(N log^2 N) なので、N が大きい場合はこちらを使う
// 例題: ABC213 F, 典型90 #006
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: SA-IS (Suffix Array - Induced Sorting) ===
// 内部用: SA-IS 本体
void sa_is_impl(const vector<int>& s, vector<int>& sa, int sigma) {
    int n = s.size();
    if (n == 1) { sa = {0}; return; }
    if (n == 2) { sa = (s[0] < s[1]) ? vector<int>{0, 1} : vector<int>{1, 0}; return; }

    // S-type / L-type の分類
    vector<bool> is_s(n);
    is_s[n - 1] = true;
    for (int i = n - 2; i >= 0; i--) {
        is_s[i] = (s[i] < s[i + 1]) || (s[i] == s[i + 1] && is_s[i + 1]);
    }

    // LMS (Left-Most S-type) の位置
    auto is_lms = [&](int i) { return i > 0 && is_s[i] && !is_s[i - 1]; };

    // バケットの境界を計算
    vector<int> bucket(sigma, 0);
    for (int c : s) bucket[c]++;

    auto get_bucket_starts = [&]() {
        vector<int> starts(sigma);
        int sum = 0;
        for (int i = 0; i < sigma; i++) { starts[i] = sum; sum += bucket[i]; }
        return starts;
    };
    auto get_bucket_ends = [&]() {
        vector<int> ends(sigma);
        int sum = 0;
        for (int i = 0; i < sigma; i++) { sum += bucket[i]; ends[i] = sum - 1; }
        return ends;
    };

    // Induced Sorting
    sa.assign(n, -1);

    // Step 1: LMS を末尾からバケットに配置
    {
        auto ends = get_bucket_ends();
        for (int i = n - 1; i >= 0; i--) {
            if (is_lms(i)) sa[ends[s[i]]--] = i;
        }
    }
    // Step 2: L-type を先頭から induced sort
    {
        auto starts = get_bucket_starts();
        for (int i = 0; i < n; i++) {
            if (sa[i] > 0 && !is_s[sa[i] - 1]) {
                sa[starts[s[sa[i] - 1]]++] = sa[i] - 1;
            }
        }
    }
    // Step 3: S-type を末尾から induced sort
    {
        auto ends = get_bucket_ends();
        for (int i = n - 1; i >= 0; i--) {
            if (sa[i] > 0 && is_s[sa[i] - 1]) {
                sa[ends[s[sa[i] - 1]]--] = sa[i] - 1;
            }
        }
    }

    // LMS部分文字列のソート結果から縮約文字列を作成
    vector<int> lms_positions;
    for (int i = 0; i < n; i++) {
        if (is_lms(i)) lms_positions.push_back(i);
    }

    // LMS部分文字列の比較
    auto lms_equal = [&](int a, int b) {
        int len = 0;
        while (true) {
            bool a_lms = (len > 0 && is_lms(a + len));
            bool b_lms = (len > 0 && is_lms(b + len));
            if (a_lms && b_lms) return s[a + len] == s[b + len];
            if (a_lms != b_lms || s[a + len] != s[b + len]) return false;
            len++;
        }
    };

    // LMS部分文字列にラベルを割り当て
    vector<int> label(n, -1);
    int cur_label = 0;
    label[sa[0]] = 0; // sentinel
    for (int i = 1; i < n; i++) {
        if (!is_lms(sa[i])) continue;
        if (!lms_equal(sa[i], sa[i - 1])) cur_label++;
        label[sa[i]] = cur_label;
    }

    // 縮約文字列
    vector<int> reduced;
    for (int i = 0; i < n; i++) {
        if (label[i] != -1) reduced.push_back(label[i]);
    }

    // 再帰的にソート
    vector<int> reduced_sa;
    if (cur_label + 1 < (int)reduced.size()) {
        sa_is_impl(reduced, reduced_sa, cur_label + 1);
    } else {
        // ラベルが全て異なる → 直接構築
        reduced_sa.resize(reduced.size());
        for (int i = 0; i < (int)reduced.size(); i++) {
            reduced_sa[reduced[i]] = i;
        }
    }

    // 再帰結果を使って最終的な SA を induced sort で構築
    sa.assign(n, -1);
    {
        auto ends = get_bucket_ends();
        for (int i = (int)reduced_sa.size() - 1; i >= 0; i--) {
            int pos = lms_positions[reduced_sa[i]];
            sa[ends[s[pos]]--] = pos;
        }
    }
    {
        auto starts = get_bucket_starts();
        for (int i = 0; i < n; i++) {
            if (sa[i] > 0 && !is_s[sa[i] - 1]) {
                sa[starts[s[sa[i] - 1]]++] = sa[i] - 1;
            }
        }
    }
    {
        auto ends = get_bucket_ends();
        for (int i = n - 1; i >= 0; i--) {
            if (sa[i] > 0 && is_s[sa[i] - 1]) {
                sa[ends[s[sa[i] - 1]]--] = sa[i] - 1;
            }
        }
    }
}

// 文字列から suffix array を構築 (O(N))
vector<int> suffix_array_sais(const string& s) {
    int n = s.size();
    vector<int> arr(n + 1);
    for (int i = 0; i < n; i++) arr[i] = s[i] + 1; // 1-indexed (0 は sentinel)
    arr[n] = 0; // sentinel
    vector<int> sa;
    sa_is_impl(arr, sa, 258);
    // sentinel (sa[0]) を除いて返す
    return vector<int>(sa.begin() + 1, sa.end());
}

// LCP配列 (Kasai's algorithm, O(N))
vector<int> lcp_array(const string& s, const vector<int>& sa) {
    int n = s.size();
    vector<int> rank_(n), lcp(n - 1);
    for (int i = 0; i < n; i++) rank_[sa[i]] = i;
    int h = 0;
    for (int i = 0; i < n; i++) {
        if (rank_[i] > 0) {
            int j = sa[rank_[i] - 1];
            while (i + h < n && j + h < n && s[i + h] == s[j + h]) h++;
            lcp[rank_[i] - 1] = h;
            if (h > 0) h--;
        } else {
            h = 0;
        }
    }
    return lcp;
}

// === 例題: 異なる部分文字列の数 ===
int main() {
    string s;
    cin >> s;
    int n = s.size();
    auto sa = suffix_array_sais(s);
    auto lcp = lcp_array(s, sa);

    long long total = (long long)n * (n + 1) / 2;
    for (int x : lcp) total -= x;
    cout << total << "\n";
    return 0;
}
