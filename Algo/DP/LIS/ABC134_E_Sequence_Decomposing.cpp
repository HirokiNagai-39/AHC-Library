// ABC134 E - Sequence Decomposing
// https://atcoder.jp/contests/abc134/tasks/abc134_e
// Decompose sequence into minimum number of strictly increasing subsequences.
// By Dilworth's theorem = length of longest non-increasing subsequence.
// Greedy with multiset. O(NlogN)
#include <bits/stdc++.h>
using namespace std;

int main(){
    int N;
    cin >> N;
    // Each "color" tracks the last element of that increasing subsequence.
    // For each A[i], find smallest element >= A[i] in the multiset (a subsequence
    // whose last element >= A[i], so we can't extend it - we need the closest one
    // from above to replace). Actually: find largest element < A[i] to extend.
    multiset<int> tails; // last elements of active subsequences
    for(int i = 0; i < N; i++){
        int a; cin >> a;
        auto it = tails.lower_bound(a);
        if(it != tails.begin()){
            --it;
            tails.erase(it);
        }
        tails.insert(a);
    }
    cout << tails.size() << endl;
}
