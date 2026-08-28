// GCD / LCM 基本
// 例題: ABC148 C - Snack
// https://atcoder.jp/contests/abc148/tasks/abc148_c
// 計算量: O(log min(a,b))
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: GCD / LCM ===
// C++17以降は std::gcd, std::lcm が使える
long long my_gcd(long long a, long long b){
    while(b){ a %= b; swap(a, b); }
    return a;
}

long long my_lcm(long long a, long long b){
    return a / my_gcd(a, b) * b; // オーバーフロー対策で先に割る
}

// N個のGCD / LCM
long long gcd_all(const vector<long long>& v){
    long long g = v[0];
    for(int i = 1; i < (int)v.size(); i++) g = my_gcd(g, v[i]);
    return g;
}

long long lcm_all(const vector<long long>& v){
    long long l = v[0];
    for(int i = 1; i < (int)v.size(); i++) l = my_lcm(l, v[i]);
    return l;
}

// === 例題: ABC148 C ===
int main(){
    long long A, B;
    cin >> A >> B;
    cout << my_lcm(A, B) << endl;
    return 0;
}
