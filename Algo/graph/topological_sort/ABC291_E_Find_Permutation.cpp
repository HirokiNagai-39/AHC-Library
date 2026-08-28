// ABC291 E - Find Permutation
// https://atcoder.jp/contests/abc291/tasks/abc291_e
// 制約からの順列復元、トポソ一意性判定
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,M;cin>>N>>M;
    vector<vector<int>>g(N);
    vector<int>deg(N,0);
    for(int i=0;i<M;i++){int a,b;cin>>a>>b;a--;b--;g[a].push_back(b);deg[b]++;}
    queue<int>q;
    for(int i=0;i<N;i++)if(deg[i]==0)q.push(i);
    vector<int>order,rank_(N);
    bool unique=true;
    while(!q.empty()){
        if(q.size()>1)unique=false;
        int v=q.front();q.pop();order.push_back(v);
        for(int u:g[v])if(--deg[u]==0)q.push(u);
    }
    if(!unique||((int)order.size()!=N)){cout<<"No"<<endl;return 0;}
    cout<<"Yes"<<endl;
    for(int i=0;i<N;i++)rank_[order[i]]=i+1;
    for(int i=0;i<N;i++)cout<<rank_[i]<<" \n"[i+1==N];
}
