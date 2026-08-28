// Warshall-Floyd: Negative cycle detection
// WFで負閉路検出。dist[i][i]<0なら負閉路
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
const ll INF=1e18;
int main(){
    int N,M;cin>>N>>M;
    vector<vector<ll>>d(N,vector<ll>(N,INF));
    for(int i=0;i<N;i++)d[i][i]=0;
    for(int i=0;i<M;i++){int a,b;ll c;cin>>a>>b>>c;a--;b--;d[a][b]=c;}
    for(int k=0;k<N;k++)for(int i=0;i<N;i++)for(int j=0;j<N;j++)
        if(d[i][k]<INF&&d[k][j]<INF)d[i][j]=min(d[i][j],d[i][k]+d[k][j]);
    bool neg=false;
    for(int i=0;i<N;i++)if(d[i][i]<0){neg=true;break;}
    if(neg)cout<<"NEGATIVE CYCLE"<<endl;
    else{
        for(int i=0;i<N;i++){
            for(int j=0;j<N;j++){
                if(j)cout<<" ";
                if(d[i][j]>=INF)cout<<"INF";else cout<<d[i][j];
            }
            cout<<"\n";
        }
    }
}
