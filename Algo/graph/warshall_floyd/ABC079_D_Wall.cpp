// ABC079 D - Wall
// https://atcoder.jp/contests/abc079/tasks/abc079_d
// 数字変換の最小コスト。10x10のWarshall-Floyd
#include <bits/stdc++.h>
using namespace std;
int main(){
    int H,W;cin>>H>>W;
    vector<vector<long long>>c(10,vector<long long>(10));
    for(int i=0;i<10;i++)for(int j=0;j<10;j++)cin>>c[i][j];
    for(int k=0;k<10;k++)for(int i=0;i<10;i++)for(int j=0;j<10;j++)
        c[i][j]=min(c[i][j],c[i][k]+c[k][j]);
    long long ans=0;
    for(int i=0;i<H;i++)for(int j=0;j<W;j++){
        int a;cin>>a;if(a!=-1&&a!=1)ans+=c[a][1];
    }
    cout<<ans<<endl;
}
