// ABC113 C - ID
// https://atcoder.jp/contests/abc113/tasks/abc113_c
// 各都道府県内で誕生年順に座圧してID付与。
// 計算量: O(N log N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M;
    cin >> N >> M;
    vector<int> P(M), Y(M);
    for (int i = 0; i < M; i++) cin >> P[i] >> Y[i];

    // 都道府県ごとに(誕生年, index)を集める
    vector<vector<pair<int,int>>> pref(N + 1);
    for (int i = 0; i < M; i++) pref[P[i]].push_back({Y[i], i});

    vector<string> ans(M);
    for (int p = 1; p <= N; p++) {
        sort(pref[p].begin(), pref[p].end());
        for (int j = 0; j < (int)pref[p].size(); j++) {
            int idx = pref[p][j].second;
            // 都道府県番号6桁 + 市番号6桁
            char buf[20];
            snprintf(buf, sizeof(buf), "%06d%06d", p, j + 1);
            ans[idx] = buf;
        }
    }
    for (auto& s : ans) cout << s << "\n";
    return 0;
}
