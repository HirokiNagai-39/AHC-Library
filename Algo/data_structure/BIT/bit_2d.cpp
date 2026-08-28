// 2D BIT (Fenwick Tree)
// 二次元BIT。点加算+矩形和。O(logH * logW)
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct BIT2D{
    int H,W;vector<vector<ll>>d;
    BIT2D(int H,int W):H(H),W(W),d(H+1,vector<ll>(W+1,0)){}
    void add(int r,int c,ll x){for(int i=r+1;i<=H;i+=i&-i)for(int j=c+1;j<=W;j+=j&-j)d[i][j]+=x;}
    ll sum(int r,int c){ll s=0;for(int i=r;i>0;i-=i&-i)for(int j=c;j>0;j-=j&-j)s+=d[i][j];return s;}
    ll sum(int r1,int c1,int r2,int c2){ // [r1,r2) x [c1,c2)
        return sum(r2,c2)-sum(r1,c2)-sum(r2,c1)+sum(r1,c1);}
};
int main(){
    int H,W,Q;cin>>H>>W>>Q;
    BIT2D bit(H,W);
    while(Q--){
        int t;cin>>t;
        if(t==1){int r,c;ll x;cin>>r>>c>>x;bit.add(r,c,x);}
        else{int r1,c1,r2,c2;cin>>r1>>c1>>r2>>c2;cout<<bit.sum(r1,c1,r2,c2)<<"\n";}
    }
}
