// 順列全探索
// next_permutation で N! 通りを全列挙
// 例題: ABC145 C - Average Length
// https://atcoder.jp/contests/abc145/tasks/abc145_c
// 計算量: O(N! * N)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    vector<double> x(N), y(N);
    for(int i = 0; i < N; i++) cin >> x[i] >> y[i];

    vector<int> perm(N);
    iota(perm.begin(), perm.end(), 0);

    double total = 0;
    int cnt = 0;

    do {
        double dist = 0;
        for(int i = 0; i + 1 < N; i++){
            double dx = x[perm[i]] - x[perm[i+1]];
            double dy = y[perm[i]] - y[perm[i+1]];
            dist += sqrt(dx*dx + dy*dy);
        }
        total += dist;
        cnt++;
    } while(next_permutation(perm.begin(), perm.end()));

    printf("%.10f\n", total / cnt);
    return 0;
}
