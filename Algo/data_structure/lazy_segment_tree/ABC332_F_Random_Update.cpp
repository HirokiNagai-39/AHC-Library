// ABC332 F - Random Update Query
// https://atcoder.jp/contests/abc332/tasks/abc332_f
// 区間代入+区間和 (mod)。遅延セグ木実戦例
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
static constexpr ll MOD=998244353;
static constexpr ll NONE=-1;
struct LazySegTree{
    int n;vector<ll>d,lz;vector<int>sz;
    LazySegTree(int n_):n(1){while(n<n_)n*=2;d.assign(2*n,0);lz.assign(2*n,NONE);sz.assign(2*n,1);
        for(int i=n-1;i>=1;i--)sz[i]=sz[2*i]+sz[2*i+1];}
    void push(int k){if(lz[k]!=NONE){for(int c:{2*k,2*k+1}){d[c]=lz[k]%MOD*sz[c]%MOD;lz[c]=lz[k];}lz[k]=NONE;}}
    void update(int l,int r,ll x){update(1,0,n,l,r,x);}
    void update(int k,int kl,int kr,int l,int r,ll x){
        if(kr<=l||r<=kl)return;if(l<=kl&&kr<=r){d[k]=x%MOD*sz[k]%MOD;lz[k]=x;return;}
        push(k);int mid=(kl+kr)/2;update(2*k,kl,mid,l,r,x);update(2*k+1,mid,kr,l,r,x);d[k]=(d[2*k]+d[2*k+1])%MOD;}
    ll query(int l,int r){return query(1,0,n,l,r);}
    ll query(int k,int kl,int kr,int l,int r){
        if(kr<=l||r<=kl)return 0;if(l<=kl&&kr<=r)return d[k];
        push(k);int mid=(kl+kr)/2;return(query(2*k,kl,mid,l,r)+query(2*k+1,mid,kr,l,r))%MOD;}
};
int main(){
    int N,M,Q;cin>>N>>M>>Q;
    LazySegTree seg(N);
    for(int i=0;i<N;i++){ll x;cin>>x;seg.update(i,i+1,x%MOD);}
    for(int i=0;i<M;i++){int l,r;ll x;cin>>l>>r>>x;l--;seg.update(l,r,x%MOD);}
    while(Q--){int l,r;cin>>l>>r;l--;cout<<seg.query(l,r)<<"\n";}
}
