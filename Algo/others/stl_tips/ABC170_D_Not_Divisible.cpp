// STL応用: set/map/multiset
// 例題: ABC170 D - Not Divisible
// https://atcoder.jp/contests/abc170/tasks/abc170_d
// A中の他のどの要素でも割り切れないものの個数
// 計算量: O(N + max(A) * log)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<int> A(N);
    for(auto&a:A) cin>>a;
    int mx=*max_element(A.begin(),A.end());
    vector<int> cnt(mx+1,0);
    for(int a:A) cnt[a]++;
    int ans=0;
    for(int a:A){
        if(cnt[a]>1) continue; // 自分と同じ値が複数ある
        bool ok=true;
        for(int d=2;d*d<=a;d++){
            if(a%d==0 && (cnt[d]>0 || cnt[a/d]>0)){ok=false;break;}
        }
        // d=1のチェック: a!=1 かつ cnt[1]>0 なら割り切れる
        if(a!=1 && cnt[1]>0) ok=false;
        if(ok) ans++;
    }
    // 正しい解法: エラトステネス的に倍数を除外
    // 上の方法はバグりやすいので、篩で解く:
    vector<bool> bad(mx+1,false);
    for(int a:A){
        for(int m=2*a;m<=mx;m+=a) bad[m]=true;
    }
    ans=0;
    for(int a:A) if(!bad[a]&&cnt[a]==1) ans++;
    cout<<ans<<endl;
}
