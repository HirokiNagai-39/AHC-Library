// ABC192 E - Train
// https://atcoder.jp/contests/abc192/tasks/abc192_e
// 電車の発車時刻制約付きDijkstra。時刻tに駅uにいるとき次発車=ceil(t/K)*K+T
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
using P=pair<ll,int>;
int main(){
    int N,M,X,Y;cin>>N>>M>>X>>Y;X--;Y--;
    struct E{int to;ll t,k;};
    vector<vector<E>>g(N);
    for(int i=0;i<M;i++){
        int a,b;ll t,k;cin>>a>>b>>t>>k;a--;b--;
        g[a].push_back({b,t,k});g[b].push_back({a,t,k});
    }
    vector<ll>d(N,1e18);
    priority_queue<P,vector<P>,greater<P>>pq;
    d[X]=0;pq.push({0,X});
    while(!pq.empty()){
        auto[c,v]=pq.top();pq.pop();
        if(c>d[v])continue;
        for(auto&[u,t,k]:g[v]){
            ll depart=((c+k-1)/k)*k;
            ll arrive=depart+t;
            if(arrive<d[u]){d[u]=arrive;pq.push({d[u],u});}
        }
    }
    cout<<(d[Y]<1e18?d[Y]:-1)<<endl;
}
