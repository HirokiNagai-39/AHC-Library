// Negative cycle detection + output vertices on cycle
// 負閉路検出+閉路上の頂点出力テンプレート
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
int main(){
    int N,M;cin>>N>>M;
    struct E{int a,b;ll w;};
    vector<E>edges(M);
    for(auto&[a,b,w]:edges){cin>>a>>b>>w;a--;b--;}
    vector<ll>d(N,0);
    vector<int>prev(N,-1);
    int last=-1;
    for(int i=0;i<N;i++){
        last=-1;
        for(auto&[a,b,w]:edges){
            if(d[a]+w<d[b]){d[b]=d[a]+w;prev[b]=a;last=b;}
        }
    }
    if(last==-1){cout<<"No negative cycle"<<endl;return 0;}
    // trace back N times to ensure we're on the cycle
    int v=last;
    for(int i=0;i<N;i++)v=prev[v];
    vector<int>cycle;
    int u=v;
    do{cycle.push_back(u);u=prev[u];}while(u!=v);
    cycle.push_back(v);
    reverse(cycle.begin(),cycle.end());
    cout<<"Negative cycle:";
    for(int x:cycle)cout<<" "<<x+1;
    cout<<endl;
}
