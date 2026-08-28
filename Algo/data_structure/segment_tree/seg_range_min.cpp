// Segment Tree: Range Minimum Query (RMQ)
// 一点更新+区間min。O(logN)
#include <bits/stdc++.h>
using namespace std;
struct SegTree{
    int n;vector<int>d;
    SegTree(int n_):n(1){while(n<n_)n*=2;d.assign(2*n,INT_MAX);}
    void update(int i,int x){i+=n;d[i]=x;while(i>1){i/=2;d[i]=min(d[2*i],d[2*i+1]);}}
    int query(int l,int r){int res=INT_MAX;for(l+=n,r+=n;l<r;l/=2,r/=2){
        if(l&1)res=min(res,d[l++]);if(r&1)res=min(res,d[--r]);}return res;}
};
int main(){
    int N,Q;cin>>N>>Q;
    SegTree seg(N);
    for(int i=0;i<N;i++){int x;cin>>x;seg.update(i,x);}
    while(Q--){
        int t;cin>>t;
        if(t==1){int i,x;cin>>i>>x;seg.update(i,x);}
        else{int l,r;cin>>l>>r;cout<<seg.query(l,r)<<"\n";}
    }
}
