// 素数列挙 / 素因数分解 / エラトステネスの篩
// 計算量: 篩 O(N log log N), 素因数分解 O(√N) or O(log N) (SPF)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: エラトステネスの篩 ===
vector<bool> sieve(int n){
    vector<bool> is_prime(n + 1, true);
    is_prime[0] = is_prime[1] = false;
    for(int i = 2; (long long)i * i <= n; i++){
        if(is_prime[i]){
            for(int j = i * i; j <= n; j += i) is_prime[j] = false;
        }
    }
    return is_prime;
}

// 素数リスト取得
vector<int> prime_list(int n){
    auto is_p = sieve(n);
    vector<int> primes;
    for(int i = 2; i <= n; i++) if(is_p[i]) primes.push_back(i);
    return primes;
}

// === ライブラリ: 最小素因数 (SPF) テーブル ===
// SPFを使うとO(log N)で素因数分解可能
vector<int> smallest_prime_factor(int n){
    vector<int> spf(n + 1);
    iota(spf.begin(), spf.end(), 0);
    for(int i = 2; (long long)i * i <= n; i++){
        if(spf[i] == i){ // iは素数
            for(int j = i * i; j <= n; j += i){
                if(spf[j] == j) spf[j] = i;
            }
        }
    }
    return spf;
}

// SPFを使った高速素因数分解
map<int, int> factorize_spf(int x, const vector<int>& spf){
    map<int, int> res;
    while(x > 1){
        res[spf[x]]++;
        x /= spf[x];
    }
    return res;
}

// === ライブラリ: 試し割り素因数分解 O(√N) ===
map<long long, int> factorize(long long n){
    map<long long, int> res;
    for(long long i = 2; i * i <= n; i++){
        while(n % i == 0){
            res[i]++;
            n /= i;
        }
    }
    if(n > 1) res[n]++;
    return res;
}

// === 例題: 約数の個数 ===
// N以下の各数の約数の個数を求める
int main(){
    int N;
    cin >> N;

    auto spf = smallest_prime_factor(N);

    for(int x = 1; x <= N; x++){
        auto f = factorize_spf(x, spf);
        long long divisor_count = 1;
        for(auto& [p, e] : f) divisor_count *= (e + 1);
        cout << x << ": " << divisor_count << "\n";
    }
    return 0;
}
