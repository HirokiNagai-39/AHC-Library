// Suffix Array (SA-IS based simple construction)
// 接尾辞配列を O(N log^2 N) で構築 (簡易版)
// 計算量: O(N log^2 N) 構築, O(M log N) 検索
#include <bits/stdc++.h>
using namespace std;

// SA構築 (O(N log^2 N))
vector<int> suffix_array(const string& s) {
    int n = s.size();
    vector<int> sa(n), rank_(n), tmp(n);
    iota(sa.begin(), sa.end(), 0);
    for (int i = 0; i < n; i++) rank_[i] = s[i];
    for (int k = 1; k < n; k <<= 1) {
        auto cmp = [&](int a, int b) {
            if (rank_[a] != rank_[b]) return rank_[a] < rank_[b];
            int ra = a+k < n ? rank_[a+k] : -1;
            int rb = b+k < n ? rank_[b+k] : -1;
            return ra < rb;
        };
        sort(sa.begin(), sa.end(), cmp);
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; i++)
            tmp[sa[i]] = tmp[sa[i-1]] + (cmp(sa[i-1], sa[i]) ? 1 : 0);
        rank_ = tmp;
        if (rank_[sa[n-1]] == n-1) break;
    }
    return sa;
}

// LCP配列 (Kasai's algorithm, O(N))
vector<int> lcp_array(const string& s, const vector<int>& sa) {
    int n = s.size();
    vector<int> rank_(n), lcp(n-1);
    for (int i = 0; i < n; i++) rank_[sa[i]] = i;
    int h = 0;
    for (int i = 0; i < n; i++) {
        if (rank_[i] > 0) {
            int j = sa[rank_[i]-1];
            while (i+h < n && j+h < n && s[i+h] == s[j+h]) h++;
            lcp[rank_[i]-1] = h;
            if (h > 0) h--;
        } else h = 0;
    }
    return lcp;
}

int main() {
    string S;
    cin >> S;
    auto sa = suffix_array(S);
    auto lcp = lcp_array(S, sa);
    int n = S.size();
    // 異なる部分文字列の数 = n*(n+1)/2 - sum(lcp)
    long long total = (long long)n*(n+1)/2;
    for (int x : lcp) total -= x;
    cout << total << endl;
    return 0;
}
