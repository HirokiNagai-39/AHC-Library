// ABC137 E - Coins Respawn
// https://atcoder.jp/contests/abc137/tasks/abc137_e
// 辺に報酬(負コスト)。1→Nの最大利益。正閉路がN到達可能ならinf
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
int main(){
    int N,M,P;cin>>N>>M>>P;
    struct E{int a,b;ll w;};
    vector<E>edges(M);
    for(auto&[a,b,w]:edges){cin>>a>>b>>w;a--;b--;w=P-w;} // minimize cost = P - coin
    // BFS reachability from 0 and to N-1
    vector<vector<int>>g(N),rg(N);
    for(auto&[a,b,w]:edges){g[a].push_back(b);rg[b].push_back(a);}
    auto bfs=[&](vector<vector<int>>&gr,int s){
        vector<bool>vis(N);queue<int>q;vis[s]=true;q.push(s);
        while(!q.empty()){int v=q.front();q.pop();for(int u:gr[v])if(!vis[u]){vis[u]=true;q.push(u);}}
        return vis;
    };
    auto from0=bfs(g,0),toN=bfs(rg,N-1);
    vector<ll>d(N,1e18);d[0]=0;
    for(int i=0;i<N-1;i++)
        for(auto&[a,b,w]:edges)if(d[a]<1e18&&d[a]+w<d[b])d[b]=d[a]+w;
    // check negative cycle on path 0->N-1
    for(auto&[a,b,w]:edges)
        if(d[a]<1e18&&d[a]+w<d[b]&&from0[a]&&toN[b]){cout<<-1<<endl;return 0;}
    cout<<max(0LL,-d[N-1])<<endl;
}
