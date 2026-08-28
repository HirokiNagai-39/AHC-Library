// ABC190 F - Shift and Inversions
// https://atcoder.jp/contests/abc190/tasks/abc190_f
// 巡回シフトと転倒数。BIT応用。先頭を末尾に移動する差分計算
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct BIT{
    int n;vector<int>d;
    BIT(int n):n(n),d(n+1,0){}
    void add(int i,int x){for(i++;i<=n;i+=i&-i)d[i]+=x;}
    int sum(int i){int s=0;for(;i>0;i-=i&-i)s+=d[i];return s;}
};
int main(){
    int N;cin>>N;
    vector<int>a(N);for(auto&x:a)cin>>x;
    // compute initial inversion count
    BIT bit(N);ll inv=0;
    for(int i=0;i<N;i++){inv+=i-bit.sum(a[i]+1);bit.add(a[i],1);}
    cout<<inv<<"\n";
    // shift: remove a[0] from front, add to back
    // removing a[i]: inversions decrease by (# of elements after a[i] that are < a[i])
    // but simpler: a[i] goes to back. It was inverted with a[i] elements > a[i] before it (=0 since it's first)
    // elements < a[i]: a[i] of them, these were NOT inversions but now become inversions
    // elements > a[i]: N-1-a[i] of them, these WERE inversions but now are not
    for(int i=0;i<N-1;i++){
        inv+=((ll)N-1-2*a[i]);
        cout<<inv<<"\n";
    }
}
