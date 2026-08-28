// Segment Tree: Point update + Range GCD query
// 一点更新+区間GCD。ABC125 C応用。O(logN)
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct SegTree{
    int n;vector<ll>d;
    SegTree(int n_):n(1){while(n<n_)n*=2;d.assign(2*n,0);}
    void update(int i,ll x){i+=n;d[i]=x;while(i>1){i/=2;d[i]=__gcd(d[2*i],d[2*i+1]);}}
    ll query(int l,int r){ll res=0;for(l+=n,r+=n;l<r;l/=2,r/=2){
        if(l&1)res=__gcd(res,d[l++]);if(r&1)res=__gcd(res,d[--r]);}return res;}
};
int main(){
    int N;cin>>N;
    SegTree seg(N);
    vector<ll>a(N);
    for(int i=0;i<N;i++){cin>>a[i];seg.update(i,a[i]);}
    // ABC125 C: max GCD when removing exactly one element
    ll ans=0;
    for(int i=0;i<N;i++){
        ll g=__gcd(seg.query(0,i),seg.query(i+1,N));
        ans=max(ans,g);
    }
    cout<<ans<<endl;
}
