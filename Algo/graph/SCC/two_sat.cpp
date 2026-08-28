// 2-SAT (SCC application)
// 含意グラフのSCCで充足可能性判定。O(V+E)
// x_i = true/false. clause (a OR b) => (!a => b) AND (!b => a)
#include <bits/stdc++.h>
using namespace std;
struct TwoSAT{
    int n;vector<vector<int>>g,rg;vector<int>order,comp;vector<bool>vis;
    TwoSAT(int n):n(n),g(2*n),rg(2*n),comp(2*n),vis(2*n){}
    // x = (2*i + val), neg(x) = x^1
    void add_clause(int i,bool f,int j,bool g_){
        // (i=f) OR (j=g_) => (!i=f => j=g_) AND (!j=g_ => i=f)
        g[2*i+!f].push_back(2*j+g_);g[2*j+!g_].push_back(2*i+f);
        rg[2*j+g_].push_back(2*i+!f);rg[2*i+f].push_back(2*j+!g_);
    }
    void dfs(int v){vis[v]=true;for(int u:g[v])if(!vis[u])dfs(u);order.push_back(v);}
    void rdfs(int v,int c){comp[v]=c;for(int u:rg[v])if(comp[u]<0)rdfs(u,c);}
    bool solve(vector<bool>&ans){
        for(int i=0;i<2*n;i++)if(!vis[i])dfs(i);
        fill(comp.begin(),comp.end(),-1);int c=0;
        for(int i=2*n-1;i>=0;i--)if(comp[order[i]]<0)rdfs(order[i],c++);
        ans.resize(n);
        for(int i=0;i<n;i++){if(comp[2*i]==comp[2*i+1])return false;ans[i]=comp[2*i]>comp[2*i+1];}
        return true;
    }
};
int main(){
    int N,M;cin>>N>>M;
    TwoSAT sat(N);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b; // 1-indexed, negative means false
        int ai=abs(a)-1,bi=abs(b)-1;
        sat.add_clause(ai,a>0,bi,b>0);
    }
    vector<bool>ans;
    if(!sat.solve(ans)){cout<<"s UNSATISFIABLE"<<endl;return 0;}
    cout<<"s SATISFIABLE\nv";
    for(int i=0;i<N;i++)cout<<" "<<(ans[i]?i+1:-(i+1));
    cout<<endl;
}
