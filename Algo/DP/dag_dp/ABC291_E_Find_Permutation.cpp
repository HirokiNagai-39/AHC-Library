// ABC291 E - Find Permutation
// https://atcoder.jp/contests/abc291/tasks/abc291_e
// Given M constraints A_i < A_{B_i} in permutation, determine if unique topo order exists.
// Unique iff at every step of topo sort, queue has exactly 1 element. O(V+E)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,M; cin>>N>>M;
    vector<vector<int>> adj(N);
    vector<int> indeg(N,0);
    for(int i=0;i<M;i++){
        int a,b; cin>>a>>b; a--;b--;
        adj[a].push_back(b);
        indeg[b]++;
    }
    queue<int> q;
    for(int i=0;i<N;i++) if(indeg[i]==0) q.push(i);
    vector<int> order;
    bool unique_flag=true;
    while(!q.empty()){
        if(q.size()>1) unique_flag=false;
        int v=q.front(); q.pop();
        order.push_back(v);
        for(int u:adj[v]) if(--indeg[u]==0) q.push(u);
    }
    if(!unique_flag){
        cout<<"No"<<endl;
    } else {
        cout<<"Yes"<<endl;
        vector<int> ans(N);
        for(int i=0;i<N;i++) ans[order[i]]=i+1;
        for(int i=0;i<N;i++) cout<<ans[i]<<" \n"[i==N-1];
    }
}
