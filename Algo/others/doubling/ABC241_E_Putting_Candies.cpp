// ABC241 E - Putting Candies
// https://atcoder.jp/contests/abc241/tasks/abc241_e
// 皿にキャンディを入れる操作をK回。A[X%N]個追加。
// ダブリングで周期検出+高速シミュレーション。
// 計算量: O(N)
#include <bits/stdc++.h>
using namespace std;

int main() {
    long long N, K;
    cin >> N >> K;
    vector<long long> A(N);
    for (auto& a : A) cin >> a;

    // 各状態(X%N)から1回の遷移を記録して周期を検出
    vector<long long> sum_at(N, -1); // sum_at[r] = r を初めて訪れた時の累積和
    vector<long long> step_at(N, -1);
    long long X = 0, steps = 0;
    vector<long long> history_sum; // step i 時点の X

    while (steps < K) {
        long long r = X % N;
        if (sum_at[r] != -1) {
            // 周期を検出
            long long cycle_start_step = step_at[r];
            long long cycle_len = steps - cycle_start_step;
            long long cycle_sum = X - sum_at[r];
            long long remaining = K - steps;
            long long full_cycles = remaining / cycle_len;
            long long leftover = remaining % cycle_len;
            X += full_cycles * cycle_sum;
            for (long long i = 0; i < leftover; i++) {
                X += A[X % N];
            }
            cout << X << endl;
            return 0;
        }
        sum_at[r] = X;
        step_at[r] = steps;
        X += A[r];
        steps++;
    }
    cout << X << endl;
    return 0;
}
