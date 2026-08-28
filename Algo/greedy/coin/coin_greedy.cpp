// 貪欲法の基本: ソートして選ぶ系
// 例題: ABC076 C - Dubious Document 2 (文字列貪欲)
// 例題: ABC131 D - Megalomania (締切ソート)
// https://atcoder.jp/contests/abc131/tasks/abc131_d
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

// === 例題: ABC131 D ===
// N個の仕事, 仕事iは所要時間A[i], 締切B[i]. 全てこなせるか
// → 締切が早い順にこなす (EDF: Earliest Deadline First)
int main(){
    int N;
    cin >> N;
    vector<pair<long long, long long>> tasks(N);
    for(int i = 0; i < N; i++) cin >> tasks[i].second >> tasks[i].first;
    // first=締切, second=所要時間 → 締切ソート

    sort(tasks.begin(), tasks.end());

    long long time = 0;
    for(auto& [deadline, work] : tasks){
        time += work;
        if(time > deadline){
            cout << "No" << endl;
            return 0;
        }
    }
    cout << "Yes" << endl;
    return 0;
}
