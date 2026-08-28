// ABC202 E - Count Descendants
// https://atcoder.jp/contests/abc202/tasks/abc202_e
// 部分木内の深さdの頂点数。ET+二分探索
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N;cin>>N;
    vector<vector<int>>g(N);
    for(int i=1;i<N;i++){int p;cin>>p;p--;g[p].push_back(i);}
    vector<int>in(N),out(N),dep(N);
    vector<vector<int>>by_depth; // by_depth[d] = sorted list of in-times
    int timer=0;
    function<void(int,int)>dfs=[&](int v,int d){
        dep[v]=d;in[v]=timer++;
        for(int u:g[v])dfs(u,d+1);
        out[v]=timer;
    };
    dfs(0,0);
    int maxd=*max_element(dep.begin(),dep.end());
    by_depth.resize(maxd+1);
    for(int i=0;i<N;i++)by_depth[dep[i]].push_back(in[i]);
    for(auto&v:by_depth)sort(v.begin(),v.end());
    int Q;cin>>Q;
    while(Q--){
        int u,d;cin>>u>>d;u--;
        auto&v=by_depth[d];
        int ans=(int)(lower_bound(v.begin(),v.end(),out[u])-lower_bound(v.begin(),v.end(),in[u]));
        cout<<ans<<"\n";
    }
}
