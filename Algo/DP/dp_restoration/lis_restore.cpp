// LIS with Restoration
// Find the Longest Increasing Subsequence and output the actual subsequence. O(NlogN)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N; cin>>N;
    vector<int> a(N);
    for(int i=0;i<N;i++) cin>>a[i];
    // dp_tail[k] = smallest tail of IS of length k+1
    vector<int> dp_tail;
    vector<int> pos(N);   // pos[i] = length of LIS ending at i (0-indexed)
    vector<int> parent(N,-1);
    vector<int> tail_idx; // index in a[] of dp_tail entries
    for(int i=0;i<N;i++){
        auto it=lower_bound(dp_tail.begin(),dp_tail.end(),a[i]);
        int k=it-dp_tail.begin();
        if(it==dp_tail.end()){
            dp_tail.push_back(a[i]);
            tail_idx.push_back(i);
        } else {
            *it=a[i];
            tail_idx[k]=i;
        }
        pos[i]=k;
        if(k>0){
            // parent: find the previous element in the LIS
            // We need to track this more carefully
        }
    }
    // Restore: find the actual subsequence by backtracking
    int lis_len=dp_tail.size();
    cout<<lis_len<<endl;
    // Collect one valid LIS by scanning from right
    vector<int> result;
    int need=lis_len-1;
    for(int i=N-1;i>=0;i--){
        if(pos[i]==need){
            result.push_back(a[i]);
            need--;
            if(need<0) break;
        }
    }
    reverse(result.begin(),result.end());
    for(int i=0;i<(int)result.size();i++)
        cout<<result[i]<<" \n"[i==(int)result.size()-1];
}
