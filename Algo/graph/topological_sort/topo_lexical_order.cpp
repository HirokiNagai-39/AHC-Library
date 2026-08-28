// Lexicographically smallest/largest topological sort
// 辞書順最小のトポソ。priority_queue(min-heap)使用
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,M;cin>>N>>M;
    vector<vector<int>>g(N);
    vector<int>deg(N,0);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);deg[b]++;}
    // lexicographically smallest
    priority_queue<int,vector<int>,greater<int>>pq;
    for(int i=0;i<N;i++)if(deg[i]==0)pq.push(i);
    vector<int>order;
    while(!pq.empty()){
        int v=pq.top();pq.pop();order.push_back(v);
        for(int u:g[v])if(--deg[u]==0)pq.push(u);
    }
    if((int)order.size()!=N){cout<<-1<<endl;return 0;}
    for(int i=0;i<N;i++)cout<<order[i]+1<<" \n"[i+1==N];
}
