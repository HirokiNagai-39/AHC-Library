// 遅延評価セグメントツリー (区間更新・区間クエリ) 汎用テンプレート
// 計算量: 構築 O(N), 更新 O(log N), クエリ O(log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: Lazy Segment Tree ===
// S:           ノードのデータ型 (モノイド)
// op(S,S)->S:  2つの子をマージする演算
// e()->S:      op の単位元
// F:           遅延作用の型
// mapping(F,S)->S:      作用 f をノード s に適用
// composition(F,F)->F:  作用の合成 (f: 新, g: 旧 → f∘g)
// id()->F:              作用の単位元 (何もしない)
template<typename S, S (*op)(S, S), S (*e)(),
         typename F, S (*mapping)(F, S), F (*composition)(F, F), F (*id)()>
struct LazySegTree {
    int n, log_;
    vector<S> dat;
    vector<F> lz;

    // --- コンストラクタ ---
    // (1) サイズ指定: 全要素を e() で初期化
    LazySegTree(int n_) : LazySegTree(vector<S>(n_, e())) {}

    // (2) 配列から構築: O(N)
    LazySegTree(const vector<S>& v){
        int n_ = v.size();
        log_ = 0; n = 1;
        while(n < n_){ n <<= 1; log_++; }
        dat.assign(2 * n, e());
        lz.assign(n, id());
        for(int i = 0; i < n_; i++) dat[n + i] = v[i];
        for(int i = n - 1; i >= 1; i--) dat[i] = op(dat[2*i], dat[2*i+1]);
    }

    void push(int k){
        if(lz[k] == id()) return;
        dat[2*k] = mapping(lz[k], dat[2*k]);
        dat[2*k+1] = mapping(lz[k], dat[2*k+1]);
        if(2*k < n){
            lz[2*k] = composition(lz[k], lz[2*k]);
            lz[2*k+1] = composition(lz[k], lz[2*k+1]);
        }
        lz[k] = id();
    }
    void push_down(int k){ for(int i = log_; i >= 1; i--) push(k >> i); }
    void push_up(int k){ while(k >>= 1) dat[k] = op(dat[2*k], dat[2*k+1]); }

    // --- set(p, x) ---
    // a[p] を x に直接代入 (遅延を解消してから書き込む)
    void set(int p, S x){
        p += n;
        push_down(p);
        dat[p] = x;
        push_up(p);
    }

    // --- get(p) ---
    // a[p] の現在値を取得 (遅延を解消してから読む)
    S get(int p){
        p += n;
        push_down(p);
        return dat[p];
    }

    // --- apply(l, r, f) ---
    // a[l], a[l+1], ..., a[r-1] に作用 f を適用 (半開区間 [l, r))
    // 具体的な意味は mapping の定義次第:
    //   加算: a[i] += f
    //   代入: a[i] = f
    //   アフィン: a[i] = f.a * a[i] + f.b
    void apply(int l, int r, F f){
        if(l >= r) return;
        l += n; r += n;
        push_down(l); push_down(r - 1);
        int l0 = l, r0 = r;
        while(l < r){
            if(l & 1){
                dat[l] = mapping(f, dat[l]);
                if(l < n) lz[l] = composition(f, lz[l]);
                l++;
            }
            if(r & 1){
                r--;
                dat[r] = mapping(f, dat[r]);
                if(r < n) lz[r] = composition(f, lz[r]);
            }
            l >>= 1; r >>= 1;
        }
        push_up(l0); push_up(r0 - 1);
    }

    // --- query(l, r) ---
    // op(a[l], a[l+1], ..., a[r-1]) を返す (半開区間 [l, r))
    S query(int l, int r){
        if(l >= r) return e();
        l += n; r += n;
        push_down(l); push_down(r - 1);
        S vl = e(), vr = e();
        while(l < r){
            if(l & 1) vl = op(vl, dat[l++]);
            if(r & 1) vr = op(dat[--r], vr);
            l >>= 1; r >>= 1;
        }
        return op(vl, vr);
    }
};

// ==========================================================
// 区間加算 + 区間和 の定義
// ==========================================================
struct S { long long sum; int size; };
S op_s(S a, S b){ return {a.sum + b.sum, a.size + b.size}; }
S e_s(){ return {0, 0}; }
bool operator==(const S&, const S&) = delete; // Sの比較は不要

using F_t = long long; // 作用 = 加算する値
S mapping_s(F_t f, S s){ return {s.sum + f * s.size, s.size}; }
F_t composition_s(F_t f, F_t g){ return f + g; }
F_t id_s(){ return 0; }

// ==========================================================
// 使い方の全例
// ==========================================================
int main(){
    // ===== 配列から構築 =====
    int N = 6;
    // a = {1, 3, 5, 7, 9, 11}
    vector<S> init(N);
    for(int i = 0; i < N; i++) init[i] = {2*i + 1, 1};

    LazySegTree<S, op_s, e_s, F_t, mapping_s, composition_s, id_s> seg(init);

    // ----- query(l, r) -----
    // 半開区間 [l, r) の和を返す
    cout << seg.query(0, 6).sum << "\n"; // 1+3+5+7+9+11 = 36
    cout << seg.query(1, 4).sum << "\n"; // 3+5+7 = 15
    cout << seg.query(0, 1).sum << "\n"; // 1 (1要素)
    cout << seg.query(3, 3).sum << "\n"; // 0 (空区間 → 単位元)

    // ----- apply(l, r, f) -----
    // [l, r) の全要素に f を作用させる
    // ここでは「区間加算」なので a[i] += f

    seg.apply(1, 4, 10);
    // a = {1, 13, 15, 17, 9, 11}
    cout << seg.query(0, 6).sum << "\n"; // 1+13+15+17+9+11 = 66
    cout << seg.query(1, 4).sum << "\n"; // 13+15+17 = 45

    seg.apply(0, 6, -1);
    // a = {0, 12, 14, 16, 8, 10}
    cout << seg.query(0, 6).sum << "\n"; // 60

    // ----- set(p, x) -----
    // a[p] を x に直接代入 (遅延を解消してから上書き)
    seg.set(2, {100, 1});
    // a = {0, 12, 100, 16, 8, 10}
    cout << seg.query(0, 6).sum << "\n"; // 0+12+100+16+8+10 = 146
    cout << seg.query(2, 3).sum << "\n"; // 100

    // ----- get(p) -----
    // a[p] の現在値を取得
    S val = seg.get(2);
    cout << val.sum << "\n";  // 100
    cout << val.size << "\n"; // 1

    val = seg.get(0);
    cout << val.sum << "\n";  // 0

    // ===== サイズ指定コンストラクタ =====
    {
        LazySegTree<S, op_s, e_s, F_t, mapping_s, composition_s, id_s> seg2(10);
        // 全要素 {0, 0} で初期化
        // ※ size=0 なので加算しても sum は変わらない
        //    使う場合は set で {value, 1} を入れる
        for(int i = 0; i < 10; i++) seg2.set(i, {0, 1});
        seg2.apply(0, 5, 3);  // a[0..5) += 3
        seg2.apply(3, 8, 7);  // a[3..8) += 7
        cout << seg2.query(0, 10).sum << "\n"; // 3*5 + 7*5 = 50
        cout << seg2.get(4).sum << "\n";       // 3 + 7 = 10
    }

    return 0;
}
