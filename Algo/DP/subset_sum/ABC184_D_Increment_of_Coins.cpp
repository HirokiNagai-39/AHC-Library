// ABC184 D - increment of coins
// https://atcoder.jp/contests/abc184/tasks/abc184_d
// 3 types of coins (gold/silver/bronze), draw one at random, ++that type.
// Expected draws until some type reaches 100. Memoization. O(100^3)
#include <bits/stdc++.h>
using namespace std;

static double memo[101][101][101];
static bool vis[101][101][101];

double solve(int a, int b, int c){
    if(a == 100 || b == 100 || c == 100) return 0;
    if(vis[a][b][c]) return memo[a][b][c];
    vis[a][b][c] = true;
    double s = a + b + c;
    memo[a][b][c] = 1.0
        + (a / s) * solve(a + 1, b, c)
        + (b / s) * solve(a, b + 1, c)
        + (c / s) * solve(a, b, c + 1);
    return memo[a][b][c];
}

int main(){
    int a, b, c;
    cin >> a >> b >> c;
    printf("%.10f\n", solve(a, b, c));
}
