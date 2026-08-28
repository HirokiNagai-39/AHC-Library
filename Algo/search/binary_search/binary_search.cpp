// 二分探索
// 例題: ABC023 D - 射撃王 (答えで二分探索)
// https://atcoder.jp/contests/abc023/tasks/abc023_d
// 計算量: O(N log(max_val))
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 二分探索テンプレート ===
// 条件を満たす最小値を求める (ok/ng の境界)
// f(x) = true となる最小の x を返す
// 前提: f(ng) = false, f(ok) = true
template<typename F>
long long binary_search(long long ng, long long ok, F f){
    while(abs(ok - ng) > 1){
        long long mid = (ng + ok) / 2;
        if(f(mid)) ok = mid;
        else ng = mid;
    }
    return ok;
}

// 実数二分探索
template<typename F>
double binary_search_real(double ng, double ok, F f, int iter = 100){
    for(int i = 0; i < iter; i++){
        double mid = (ng + ok) / 2;
        if(f(mid)) ok = mid;
        else ng = mid;
    }
    return ok;
}

// === 例題: ABC023 D ===
// N個の風船, 高さH[i], 速さS[i]. 時刻tでの高さ = H[i] + S[i]*t
// 全ての風船を高さX以下で割る. 1時刻に1個しか割れない
// 可能な最小のXを求める
int main(){
    int N;
    cin >> N;
    vector<long long> H(N), S(N);
    for(int i = 0; i < N; i++) cin >> H[i] >> S[i];

    auto check = [&](long long X) -> bool {
        // 各風船の「制限時刻」= (X - H[i]) / S[i]
        vector<long long> deadline(N);
        for(int i = 0; i < N; i++){
            if(H[i] > X) return false;
            deadline[i] = (X - H[i]) / S[i];
        }
        // 制限時刻が早い順に割る
        sort(deadline.begin(), deadline.end());
        for(int i = 0; i < N; i++){
            if(deadline[i] < i) return false;
        }
        return true;
    };

    cout << binary_search(-1LL, 2e18, check) << endl;
    return 0;
}
