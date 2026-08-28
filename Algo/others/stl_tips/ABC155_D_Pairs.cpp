// STL応用: lower_bound/upper_bound + 二分探索
// 例題: ABC155 D - Pairs
// https://atcoder.jp/contests/abc155/tasks/abc155_d
// N個の整数からペアの積でK番目に小さいもの
// 計算量: O(N log N * log(max))
#include <bits/stdc++.h>
using namespace std;
int main(){
    long long N,K; cin>>N>>K;
    vector<long long> a(N);
    for(auto&x:a) cin>>x;
    sort(a.begin(),a.end());
    // 積≤xのペア数を数える関数
    auto count_le=[&](long long x)->long long{
        long long cnt=0;
        for(int i=0;i<N;i++){
            if(a[i]>=0){
                // a[i]*a[j]<=x → a[j]<=x/a[i]
                long long hi=x/a[i]; // floor
                if(a[i]>0) cnt+=upper_bound(a.begin()+i+1,a.end(),hi)-a.begin()-i-1;
                else cnt+=N-i-1; // a[i]==0, x>=0 なら全部OK
            } else {
                // a[i]<0, a[i]*a[j]<=x → a[j]>=ceil(x/a[i])
                long long lo;
                if(x>=0) lo=x/a[i]; // negative / negative
                else lo=(x-a[i]-1)/a[i]; // ceiling division
                // 正確にはa[j]>= x/a[i]のceil
                // a[i]<0のとき a[j]>=ceil(x/a[i])
                lo=(-(-x+a[i]+1)/(-a[i])); // 安全なceiling
                // ↑ 複雑なので別方式:
                // 実数でx/a[i]以上のa[j]の個数
                // a[i]<0 → 不等号反転してa[j]>=x/a[i](切り上げ)
            }
        }
        return cnt;
    };
    // 簡潔な正解版
    auto count_pairs=[&](long long mid)->long long{
        long long cnt=0;
        for(int i=0;i<N;i++){
            if(a[i]>0){
                // a[j]<=mid/a[i]
                long long t=mid/a[i]-(mid%a[i]<0?1:0);
                int pos=upper_bound(a.begin()+i+1,a.end(),t)-a.begin();
                cnt+=pos-i-1;
            } else if(a[i]<0){
                // a[j]>=ceil(mid/a[i])
                long long t;
                if(mid>=0) t=-((-mid)/(-a[i]));
                else t=(-mid+(-a[i])-1)/(-a[i]);
                int pos=lower_bound(a.begin()+i+1,a.end(),t)-a.begin();
                cnt+=N-pos;
            } else {
                if(mid>=0) cnt+=N-i-1;
            }
        }
        return cnt;
    };
    long long lo=-1e18,hi=1e18;
    while(hi-lo>1){
        long long mid=lo+(hi-lo)/2;
        if(count_pairs(mid)>=K) hi=mid;
        else lo=mid;
    }
    cout<<hi<<endl;
}
