// Union-Find (素集合データ構造)
// 例題: ATC001 B - Union Find
// https://atcoder.jp/contests/atc001/tasks/unionfind_a
// 計算量: ほぼ O(α(N)) ≒ O(1)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Union-Find ===
struct UnionFind {
    vector<int> par, rank_, size_;

    UnionFind(int n) : par(n), rank_(n, 0), size_(n, 1) {
        iota(par.begin(), par.end(), 0);
    }

    int find(int x){
        if(par[x] != x) par[x] = find(par[x]); // 経路圧縮
        return par[x];
    }

    bool unite(int x, int y){
        x = find(x); y = find(y);
        if(x == y) return false;
        if(rank_[x] < rank_[y]) swap(x, y); // union by rank
        par[y] = x;
        size_[x] += size_[y];
        if(rank_[x] == rank_[y]) rank_[x]++;
        return true;
    }

    bool same(int x, int y){ return find(x) == find(y); }
    int size(int x){ return size_[find(x)]; }

    // 連結成分の数
    int count_groups(){
        int cnt = 0;
        for(int i = 0; i < (int)par.size(); i++){
            if(find(i) == i) cnt++;
        }
        return cnt;
    }

    // 全連結成分を列挙
    vector<vector<int>> groups(){
        int n = par.size();
        vector<vector<int>> res(n);
        for(int i = 0; i < n; i++) res[find(i)].push_back(i);
        vector<vector<int>> ret;
        for(auto& v : res) if(!v.empty()) ret.push_back(v);
        return ret;
    }
};

// === 重み付きUnion-Find ===
// weight(y) - weight(x) = w の関係を管理
template<typename T>
struct WeightedUnionFind {
    vector<int> par, rank_;
    vector<T> diff_; // 親との重みの差

    WeightedUnionFind(int n) : par(n), rank_(n, 0), diff_(n, 0) {
        iota(par.begin(), par.end(), 0);
    }

    int find(int x){
        if(par[x] == x) return x;
        int r = find(par[x]);
        diff_[x] += diff_[par[x]];
        return par[x] = r;
    }

    T weight(int x){ find(x); return diff_[x]; }

    // weight(y) - weight(x) = w
    bool unite(int x, int y, T w){
        w += weight(x) - weight(y);
        x = find(x); y = find(y);
        if(x == y) return false;
        if(rank_[x] < rank_[y]){ swap(x, y); w = -w; }
        par[y] = x;
        diff_[y] = w;
        if(rank_[x] == rank_[y]) rank_[x]++;
        return true;
    }

    // weight(y) - weight(x)
    T diff(int x, int y){ return weight(y) - weight(x); }
    bool same(int x, int y){ return find(x) == find(y); }
};

// === 例題: ATC001 B ===
int main(){
    int N, Q;
    cin >> N >> Q;
    UnionFind uf(N);
    while(Q--){
        int P, A, B;
        cin >> P >> A >> B;
        if(P == 0){
            uf.unite(A, B);
        } else {
            cout << (uf.same(A, B) ? "Yes" : "No") << "\n";
        }
    }
    return 0;
}
