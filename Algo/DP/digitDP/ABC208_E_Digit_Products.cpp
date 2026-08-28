// ABC208 E - Digit Products
// https://atcoder.jp/contests/abc208/tasks/abc208_e
// Count numbers in [1,N] where product of digits <= K. Digit DP.
// Key: product of digits can be large, but only has limited distinct values
// (products of digits 0-9), so use map for states.
#include <bits/stdc++.h>
using namespace std;
int main(){
    long long N,K; cin>>N>>K;
    string s=to_string(N);
    int n=s.size();
    // dp with memoization using map: (pos, prod, tight, started)
    map<tuple<int,long long,bool,bool>,long long> memo;
    function<long long(int,long long,bool,bool)> dp=[&](int pos,long long prod,bool tight,bool started)->long long{
        if(prod>K&&started) return 0;
        if(pos==n) return started?1:0;
        auto key=make_tuple(pos,prod,tight,started);
        if(memo.count(key)) return memo[key];
        int lim=tight?(s[pos]-'0'):9;
        long long res=0;
        for(int d=0;d<=lim;d++){
            if(!started&&d==0)
                res+=dp(pos+1,0,tight&&(d==lim),false);
            else
                res+=dp(pos+1,(started?prod:1)*d,tight&&(d==lim),true);
        }
        return memo[key]=res;
    };
    cout<<dp(0,0,true,false)<<endl;
}
