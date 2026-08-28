// 区間スケジューリング問題 (貪欲法の典型)
// 例題: ABC103 D - Islands War (区間版)
// https://atcoder.jp/contests/keyence2020/tasks/keyence2020_b (類題)
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

// === ライブラリ: 区間スケジューリング ===
// N個の区間 [l, r) から重ならない最大個数を選ぶ
// → 終了時刻でソートして貪欲
int interval_scheduling(vector<pair<int,int>>& intervals){
    // 終了時刻でソート
    sort(intervals.begin(), intervals.end(), [](const auto& a, const auto& b){
        return a.second < b.second;
    });
    int count = 0;
    int last_end = -1;
    for(auto& [l, r] : intervals){
        if(l >= last_end){
            count++;
            last_end = r;
        }
    }
    return count;
}

// === 例題: 仕事選び ===
// N個の仕事 [開始, 終了). 同時に1つしかできない. 最大何個できるか
int main(){
    int N;
    cin >> N;
    vector<pair<int,int>> jobs(N);
    for(int i = 0; i < N; i++) cin >> jobs[i].first >> jobs[i].second;

    cout << interval_scheduling(jobs) << endl;
    return 0;
}
