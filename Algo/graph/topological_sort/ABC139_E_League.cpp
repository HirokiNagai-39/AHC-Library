// ABC139 E - League
// https://atcoder.jp/contests/abc139/tasks/abc139_e
// 試合順序の依存関係→トポソで最長パス=答え
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N;cin>>N;
    vector<vector<int>>A(N,vector<int>(N-1));
    for(int i=0;i<N;i++)for(int j=0;j<N-1;j++){cin>>A[i][j];A[i][j]--;}
    // match (i, A[i][j]) must happen before (i, A[i][j+1])
    // encode match (min,max) -> id
    map<pair<int,int>,int>mid;
    int cnt=0;
    for(int i=0;i<N;i++)for(int j=0;j<N-1;j++){
        auto p=make_pair(min(i,A[i][j]),max(i,A[i][j]));
        if(!mid.count(p))mid[p]=cnt++;
    }
    int V=cnt;
    vector<vector<int>>g(V);
    vector<int>deg(V,0);
    for(int i=0;i<N;i++)for(int j=0;j+1<N-1;j++){
        auto p1=make_pair(min(i,A[i][j]),max(i,A[i][j]));
        auto p2=make_pair(min(i,A[i][j+1]),max(i,A[i][j+1]));
        g[mid[p1]].push_back(mid[p2]);deg[mid[p2]]++;
    }
    queue<int>q;vector<int>dist(V,0);
    for(int i=0;i<V;i++)if(deg[i]==0)q.push(i);
    int processed=0,ans=0;
    while(!q.empty()){
        int v=q.front();q.pop();processed++;ans=max(ans,dist[v]);
        for(int u:g[v]){dist[u]=max(dist[u],dist[v]+1);if(--deg[u]==0)q.push(u);}
    }
    cout<<(processed==V?ans+1:-1)<<endl;
}
