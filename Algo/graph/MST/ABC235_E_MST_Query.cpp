// ABC235 E - MST+1
// https://atcoder.jp/contests/abc235/tasks/abc235_e
// 辺追加クエリでMSTに含まれるか判定。オフラインでKruskal
#include <bits/stdc++.h>
using namespace std;
struct UF{
    vector<int>p,r;UF(int n):p(n),r(n,0){iota(p.begin(),p.end(),0);}
    int find(int x){return p[x]==x?x:p[x]=find(p[x]);}
    bool unite(int a,int b){a=find(a);b=find(b);if(a==b)return false;
        if(r[a]<r[b])swap(a,b);p[b]=a;if(r[a]==r[b])r[a]++;return true;}
    bool same(int a,int b){return find(a)==find(b);}
};
int main(){
    int N,M,Q;cin>>N>>M>>Q;
    // edges: (w, type, idx) type=0 for graph edge, type=1 for query
    vector<tuple<int,int,int,int>>all; // (w, type, u, v_or_idx)
    for(int i=0;i<M;i++){int a,b,c;cin>>a>>b>>c;a--;b--;all.push_back({c,0,a,b});}
    vector<tuple<int,int,int>>queries(Q);
    for(int i=0;i<Q;i++){int a,b,c;cin>>a>>b>>c;a--;b--;queries[i]={c,a,b};all.push_back({c,1,i,0});}
    // sort by weight, graph edges first for same weight
    sort(all.begin(),all.end(),[](auto&a,auto&b){
        if(get<0>(a)!=get<0>(b))return get<0>(a)<get<0>(b);
        return get<1>(a)<get<1>(b);
    });
    UF uf(N);
    vector<string>ans(Q);
    for(auto&[w,t,x,y]:all){
        if(t==0){uf.unite(x,y);}
        else{auto[c,u,v]=queries[x];ans[x]=uf.same(u,v)?"No":"Yes";}
    }
    for(auto&s:ans)cout<<s<<"\n";
}
