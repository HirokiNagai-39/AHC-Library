// Segment Tree: Point update + Range sum query
// 一点更新+区間和。O(logN)
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct SegTree{
    int n;vector<ll>d;
    SegTree(int n_):n(1){while(n<n_)n*=2;d.assign(2*n,0);}
    void update(int i,ll x){i+=n;d[i]=x;while(i>1){i/=2;d[i]=d[2*i]+d[2*i+1];}}
    void add(int i,ll x){update(i,d[i+n]+x);}
    ll query(int l,int r){ll res=0;for(l+=n,r+=n;l<r;l/=2,r/=2){
        if(l&1)res+=d[l++];if(r&1)res+=d[--r];}return res;}
};
int main(){
    int N,Q;cin>>N>>Q;
    SegTree seg(N);
    for(int i=0;i<N;i++){ll x;cin>>x;seg.update(i,x);}
    while(Q--){
        int t;cin>>t;
        if(t==1){int i;ll x;cin>>i>>x;seg.add(i,x);}
        else{int l,r;cin>>l>>r;cout<<seg.query(l,r)<<"\n";}
    }
}
