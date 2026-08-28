// ABC120 D - Decayed Bridges
// https://atcoder.jp/contests/abc120/tasks/abc120_d
// 逆順Union-Find。辺を逆に追加して連結成分管理
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct UF{
    vector<int>p,sz;int comp;
    UF(int n):p(n),sz(n,1),comp(n){iota(p.begin(),p.end(),0);}
    int find(int x){return p[x]==x?x:p[x]=find(p[x]);}
    bool unite(int a,int b){a=find(a);b=find(b);if(a==b)return false;
        if(sz[a]<sz[b])swap(a,b);p[b]=a;sz[a]+=sz[b];comp--;return true;}
};
int main(){
    int N,M;cin>>N>>M;
    vector<int>A(M),B(M);
    for(int i=0;i<M;i++){cin>>A[i]>>B[i];A[i]--;B[i]--;}
    UF uf(N);
    // inconvenience = N*(N-1)/2 - sum of C(sz,2) for each component
    // process edges in reverse
    vector<ll>ans(M);
    ll total=(ll)N*(N-1)/2;
    for(int i=M-1;i>=0;i--){
        ans[i]=total;
        int a=uf.find(A[i]),b=uf.find(B[i]);
        if(a!=b){
            total-=(ll)uf.sz[a]*(uf.sz[a]-1)/2+(ll)uf.sz[b]*(uf.sz[b]-1)/2;
            uf.unite(A[i],B[i]);
            int c=uf.find(A[i]);
            total+=(ll)uf.sz[c]*(uf.sz[c]-1)/2;
        }
    }
    for(int i=0;i<M;i++)cout<<ans[i]<<"\n";
}
