// ABC285 D - Change Usernames
// https://atcoder.jp/contests/abc285/tasks/abc285_d
// ユーザ名変更の循環依存判定。閉路があればNo
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N;cin>>N;
    map<string,int>id;int cnt=0;
    auto getId=[&](const string&s){if(!id.count(s))id[s]=cnt++;return id[s];};
    vector<pair<int,int>>edges(N);
    for(int i=0;i<N;i++){
        string s,t;cin>>s>>t;
        edges[i]={getId(s),getId(t)};
    }
    int V=cnt;
    vector<vector<int>>g(V);
    vector<int>deg(V,0);
    for(auto[s,t]:edges){g[t].push_back(s);deg[s]++;} // t->s: t must be freed before s takes it
    // topological sort to check cycle
    queue<int>q;
    for(int i=0;i<V;i++)if(deg[i]==0)q.push(i);
    int processed=0;
    while(!q.empty()){int v=q.front();q.pop();processed++;for(int u:g[v])if(--deg[u]==0)q.push(u);}
    cout<<(processed==V?"Yes":"No")<<endl;
}
