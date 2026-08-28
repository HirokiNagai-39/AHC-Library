// ABC191 E - Come Back Quickly
// https://atcoder.jp/contests/abc191/tasks/abc191_e
// 各頂点から出発して戻る最短閉路。各頂点でDijkstra
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
using P=pair<ll,int>;
int main(){
    int N,M;cin>>N>>M;
    vector<vector<P>>g(N);
    for(int i=0;i<M;i++){int a,b;ll c;cin>>a>>b>>c;a--;b--;g[a].push_back({c,b});}
    for(int s=0;s<N;s++){
        vector<ll>d(N,1e18);
        priority_queue<P,vector<P>,greater<P>>pq;
        // don't set d[s]=0; push neighbors directly
        for(auto[w,u]:g[s]){if(w<d[u]){d[u]=w;pq.push({w,u});}}
        while(!pq.empty()){
            auto[c,v]=pq.top();pq.pop();
            if(c>d[v])continue;
            for(auto[w,u]:g[v])if(d[v]+w<d[u]){d[u]=d[v]+w;pq.push({d[u],u});}
        }
        cout<<(d[s]<1e18?d[s]:-1)<<"\n";
    }
}
