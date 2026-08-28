// BIT: k-th smallest element via binary search on BIT
// BIT上の二分探索でk番目の要素。O(logN)
#include <bits/stdc++.h>
using namespace std;
struct BIT{
    int n;vector<int>d;
    BIT(int n):n(n),d(n+1,0){}
    void add(int i,int x){for(i++;i<=n;i+=i&-i)d[i]+=x;}
    // find smallest i such that sum[0..i] >= k (1-indexed k)
    int kth(int k){
        int pos=0;
        for(int pw=1<<(__lg(n));pw;pw>>=1){
            if(pos+pw<=n&&d[pos+pw]<k){pos+=pw;k-=d[pos];}
        }
        return pos; // 0-indexed
    }
};
int main(){
    int N,Q;cin>>N>>Q;
    BIT bit(N);
    while(Q--){
        int t;cin>>t;
        if(t==1){int x;cin>>x;bit.add(x,1);} // insert x
        else if(t==2){int x;cin>>x;bit.add(x,-1);} // remove x
        else{int k;cin>>k;cout<<bit.kth(k)<<"\n";} // k-th smallest
    }
}
