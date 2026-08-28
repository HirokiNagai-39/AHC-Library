// ABC177 D - Friends
// https://atcoder.jp/contests/abc177/tasks/abc177_d
// 友達グループの最大サイズ = 答え(同グループから1人ずつ選ぶ)
#include <bits/stdc++.h>
using namespace std;
struct UF{
    vector<int>p,sz;
    UF(int n):p(n),sz(n,1){iota(p.begin(),p.end(),0);}
    int find(int x){return p[x]==x?x:p[x]=find(p[x]);}
    void unite(int a,int b){a=find(a);b=find(b);if(a==b)return;
        if(sz[a]<sz[b])swap(a,b);p[b]=a;sz[a]+=sz[b];}
};
int main(){
    int N,M;cin>>N>>M;
    UF uf(N);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b;a--;b--;uf.unite(a,b);}
    int ans=0;
    for(int i=0;i<N;i++)if(uf.find(i)==i)ans=max(ans,uf.sz[i]);
    cout<<ans<<endl;
}
