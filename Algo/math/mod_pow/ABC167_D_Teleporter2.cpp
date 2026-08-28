// ABC167 D - Teleporter
// https://atcoder.jp/contests/abc167/tasks/abc167_d
// 町1からK回テレポートした先。Functional graphの周期検出。
// ダブリングではなくサイクル検出で解く(mod_powと同原理)。
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long K;
    cin >> N >> K;
    vector<int> A(N);
    for (auto& a : A) { cin >> a; a--; }

    // 町0から辿って経路を記録し、サイクルを検出
    vector<int> path;
    vector<int> visited(N, -1);
    int cur = 0;
    for (int i = 0; ; i++) {
        if (visited[cur] != -1) {
            int cycle_start = visited[cur];
            int cycle_len = i - cycle_start;
            if (K < cycle_start) {
                cout << path[K] + 1 << endl;
            } else {
                long long rem = (K - cycle_start) % cycle_len;
                cout << path[cycle_start + rem] + 1 << endl;
            }
            return 0;
        }
        visited[cur] = i;
        path.push_back(cur);
        cur = A[cur];
    }
}
