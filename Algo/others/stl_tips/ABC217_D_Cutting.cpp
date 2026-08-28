// STL応用: set の活用
// 例題: ABC217 D - Cutting
// https://atcoder.jp/contests/abc217/tasks/abc217_d
// L cmの木材にカット/長さクエリ。setでカット位置を管理
// 計算量: O(Q log Q)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int L,Q; cin>>L>>Q;
    set<int> cuts;
    cuts.insert(0);
    cuts.insert(L);
    while(Q--){
        int c,x; cin>>c>>x;
        if(c==1){
            cuts.insert(x);
        } else {
            auto it=cuts.upper_bound(x);
            int r=*it;
            --it;
            int l=*it;
            cout<<r-l<<"\n";
        }
    }
}
