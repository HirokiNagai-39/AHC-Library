// ABC184 D - increment of coins
// https://atcoder.jp/contests/abc184/tasks/abc184_d
// 3 types of coins (gold, silver, bronze). Draw random coin, duplicate it.
// Expected draws until some type reaches 100. Memoized recursion. O(100^3)
#include <bits/stdc++.h>
using namespace std;
double memo[101][101][101];
bool vis[101][101][101];
double solve(int a,int b,int c){
    if(a==100||b==100||c==100) return 0;
    if(vis[a][b][c]) return memo[a][b][c];
    vis[a][b][c]=true;
    double total=a+b+c;
    double res=0;
    res+=(a/total)*(1+solve(a+1,b,c));
    res+=(b/total)*(1+solve(a,b+1,c));
    res+=(c/total)*(1+solve(a,b,c+1));
    return memo[a][b][c]=res;
}
int main(){
    int A,B,C; cin>>A>>B>>C;
    memset(vis,false,sizeof(vis));
    printf("%.9f\n",solve(A,B,C));
}
