// BIT (Fenwick Tree): Point add + Range sum query
// BIT基本: 一点加算+区間和クエリ。O(logN)
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct BIT{
    int n;vector<ll>d;
    BIT(int n):n(n),d(n+1,0){}
    void add(int i,ll x){for(i++;i<=n;i+=i&-i)d[i]+=x;}
    ll sum(int i){ll s=0;for(;i>0;i-=i&-i)s+=d[i];return s;} // [0,i)
    ll sum(int l,int r){return sum(r)-sum(l);} // [l,r)
};
int main(){
    int N,Q;cin>>N>>Q;
    BIT bit(N);
    for(int i=0;i<N;i++){ll x;cin>>x;bit.add(i,x);}
    while(Q--){
        int t;cin>>t;
        if(t==1){int i;ll x;cin>>i>>x;bit.add(i,x);}
        else{int l,r;cin>>l>>r;cout<<bit.sum(l,r)<<"\n";}
    }
}
