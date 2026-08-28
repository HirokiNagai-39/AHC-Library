// ABC286 E - Souvenir
// https://atcoder.jp/contests/abc286/tasks/abc286_e
// 最短路+最大スコア。WF変形。距離最小、同距離ならスコア最大
#include <bits/stdc++.h>
using namespace std;
using ll=long long;
const ll INF=1e18;
int main(){
    int N;cin>>N;
    vector<ll>A(N);for(auto&a:A)cin>>a;
    vector<vector<ll>>d(N,vector<ll>(N,INF));
    vector<vector<ll>>sc(N,vector<ll>(N,0)); // max score
    string s;
    for(int i=0;i<N;i++){
        cin>>s;d[i][i]=0;sc[i][i]=A[i];
        for(int j=0;j<N;j++)if(s[j]=='Y'){d[i][j]=1;sc[i][j]=A[i]+A[j];}
    }
    for(int k=0;k<N;k++)for(int i=0;i<N;i++)for(int j=0;j<N;j++){
        if(d[i][k]>=INF||d[k][j]>=INF)continue;
        ll nd=d[i][k]+d[k][j],ns=sc[i][k]+sc[k][j]-A[k];
        if(nd<d[i][j]||(nd==d[i][j]&&ns>sc[i][j])){d[i][j]=nd;sc[i][j]=ns;}
    }
    int Q;cin>>Q;
    while(Q--){int u,v;cin>>u>>v;u--;v--;
        if(d[u][v]>=INF)cout<<"Impossible\n";
        else cout<<d[u][v]<<" "<<sc[u][v]<<"\n";
    }
}
