// Functional Graph (各頂点出次数1) のダブリング + 周期検出
// Floyd's cycle detection で周期を求め、ダブリングでK回先を高速計算。
// 計算量: O(N) 周期検出, O(N log K) ダブリング前処理
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long K;
    cin >> N >> K;
    vector<int> f(N);
    for (auto& x : f) cin >> x;

    // Floyd's cycle detection from vertex 0
    int slow = f[0], fast = f[f[0]];
    while (slow != fast) { slow = f[slow]; fast = f[f[fast]]; }

    // cycle length
    int cycle_len = 1;
    int tmp = f[slow];
    while (tmp != slow) { tmp = f[tmp]; cycle_len++; }

    // distance to cycle entry
    slow = 0; fast = f[f[0]]; // restart
    // Re-do Floyd correctly
    slow = 0;
    int cur = 0;
    // Simple approach: trace from 0, find entry
    vector<int> path;
    vector<int> visited(N, -1);
    cur = 0;
    for (int i = 0; ; i++) {
        if (visited[cur] != -1) {
            int cs = visited[cur];
            int cl = i - cs;
            if (K < cs) { cout << path[K] << endl; return 0; }
            long long rem = (K - cs) % cl;
            cout << path[cs + rem] << endl;
            return 0;
        }
        visited[cur] = i;
        path.push_back(cur);
        cur = f[cur];
    }
}
