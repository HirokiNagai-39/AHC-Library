// Weighted Union-Find (Potentialled Union-Find)
// 重み付きUF例題。weight(x)-weight(root)を管理。diff(x,y)=weight(x)-weight(y)
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct WeightedUF{
    vector<int>p,rank_;vector<ll>w; // w[x] = weight(x) - weight(parent(x))
    WeightedUF(int n):p(n),rank_(n,0),w(n,0){iota(p.begin(),p.end(),0);}
    int find(int x){if(p[x]==x)return x;int r=find(p[x]);w[x]+=w[p[x]];return p[x]=r;}
    ll weight(int x){find(x);return w[x];}
    ll diff(int x,int y){return weight(x)-weight(y);} // weight(x)-weight(y)
    bool unite(int x,int y,ll d){ // weight(x)-weight(y)=d
        d+=weight(y)-weight(x);
        x=find(x);y=find(y);
        if(x==y)return false; // already same (check consistency with d==0)
        if(rank_[x]<rank_[y]){swap(x,y);d=-d;}
        p[y]=x;w[y]=d;
        if(rank_[x]==rank_[y])rank_[x]++;
        return true;
    }
    bool same(int x,int y){return find(x)==find(y);}
};
int main(){
    int N,Q;cin>>N>>Q;
    WeightedUF uf(N);
    while(Q--){
        int t;cin>>t;
        if(t==0){int x,y;ll w;cin>>x>>y>>w; // weight(x)-weight(y)=w
            if(uf.same(x,y)){cout<<(uf.diff(x,y)==w?"Yes":"No")<<"\n";}
            else{uf.unite(x,y,w);cout<<"Yes\n";}
        }else{int x,y;cin>>x>>y;
            if(!uf.same(x,y))cout<<"?\n";
            else cout<<uf.diff(x,y)<<"\n";
        }
    }
}
