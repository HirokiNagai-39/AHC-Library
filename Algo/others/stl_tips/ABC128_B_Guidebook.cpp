// STL応用: カスタムソート / stable_sort / pair
// 例題: ABC128 B - Guidebook
// https://atcoder.jp/contests/abc128/tasks/abc128_b
// 市ごとにグループ化し, 評価降順ソート
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<tuple<string,int,int>> v(N); // (市名, -評価, 元index)
    for(int i=0;i<N;i++){
        string s; int p; cin>>s>>p;
        v[i]={s,-p,i+1};
    }
    sort(v.begin(),v.end());
    for(auto&[s,p,id]:v) cout<<id<<"\n";
}
