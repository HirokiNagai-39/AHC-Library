// Count the number of Longest Increasing Subsequences
// Given a sequence, find both LIS length and the count of distinct LIS. O(NlogN)
// Uses BIT (Fenwick tree) for range max query with count.
#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1000000007;

struct BIT {
    int n;
    vector<pair<int,long long>> tree; // (max_len, count)
    BIT(int n) : n(n), tree(n + 1, {0, 1}) {}

    void update(int i, int len, long long cnt){
        for(; i <= n; i += i & (-i)){
            if(tree[i].first < len){
                tree[i] = {len, cnt};
            } else if(tree[i].first == len){
                tree[i].second = (tree[i].second + cnt) % MOD;
            }
        }
    }

    pair<int,long long> query(int i){
        pair<int,long long> res = {0, 1};
        for(; i > 0; i -= i & (-i)){
            if(tree[i].first > res.first){
                res = tree[i];
            } else if(tree[i].first == res.first){
                res.second = (res.second + tree[i].second) % MOD;
            }
        }
        return res;
    }
};

int main(){
    int N;
    cin >> N;
    vector<int> A(N);
    for(auto &x : A) cin >> x;

    // Coordinate compress
    vector<int> sorted_A = A;
    sort(sorted_A.begin(), sorted_A.end());
    sorted_A.erase(unique(sorted_A.begin(), sorted_A.end()), sorted_A.end());
    int M = sorted_A.size();
    for(auto &x : A)
        x = lower_bound(sorted_A.begin(), sorted_A.end(), x) - sorted_A.begin() + 1;

    BIT bit(M);
    int lis_len = 0;
    long long lis_cnt = 0;
    for(int i = 0; i < N; i++){
        auto [len, cnt] = bit.query(A[i] - 1);
        len++;
        bit.update(A[i], len, cnt);
        if(len > lis_len){ lis_len = len; lis_cnt = cnt; }
        else if(len == lis_len){ lis_cnt = (lis_cnt + cnt) % MOD; }
    }
    cout << lis_len << " " << lis_cnt << endl;
}
