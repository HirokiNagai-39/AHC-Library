// バックトラッキング (DFS応用)
// 再帰で選択肢を試し, 条件を満たさなければ戻る
// 例題: ABC015 D - 高橋くんの苦悩 (DFS解法), N-Queen等
// 計算量: 問題依存 (枝刈りで高速化)
#include <bits/stdc++.h>
using namespace std;

// ============================================================
// パターン1: 部分和問題
// 数列から部分集合を選び, 和がtargetになるか
// 計算量: O(2^N) (最悪), 枝刈りで高速化
// ============================================================
bool subset_sum(const vector<int>& a, int idx, int target){
    if(target == 0) return true;
    if(idx == (int)a.size() || target < 0) return false;
    // a[idx]を使う or 使わない
    return subset_sum(a, idx + 1, target - a[idx]) ||
           subset_sum(a, idx + 1, target);
}

// ============================================================
// パターン2: 順列生成 (重複なし)
// N個からK個選んで並べる
// ============================================================
void gen_permutation(int N, int K, vector<int>& chosen, vector<bool>& used,
                     vector<vector<int>>& result){
    if((int)chosen.size() == K){
        result.push_back(chosen);
        return;
    }
    for(int i = 0; i < N; i++){
        if(used[i]) continue;
        used[i] = true;
        chosen.push_back(i);
        gen_permutation(N, K, chosen, used, result);
        chosen.pop_back();
        used[i] = false;
    }
}

// ============================================================
// パターン3: 組み合わせ生成
// N個からK個選ぶ (順序なし)
// ============================================================
void gen_combination(int N, int K, int start, vector<int>& chosen,
                     vector<vector<int>>& result){
    if((int)chosen.size() == K){
        result.push_back(chosen);
        return;
    }
    for(int i = start; i < N; i++){
        chosen.push_back(i);
        gen_combination(N, K, i + 1, chosen, result);
        chosen.pop_back();
    }
}

// ============================================================
// パターン4: N-Queen
// N*Nの盤面にN個のクイーンを互いに攻撃しないように配置
// 計算量: O(N!) 程度
// ============================================================
int n_queen(int N){
    int count = 0;
    vector<int> col(N); // col[i] = i行目のクイーンの列
    vector<bool> used_col(N, false);
    vector<bool> used_diag1(2 * N, false); // 右下がり対角線 (row+col)
    vector<bool> used_diag2(2 * N, false); // 右上がり対角線 (row-col+N)

    function<void(int)> solve = [&](int row){
        if(row == N){
            count++;
            return;
        }
        for(int c = 0; c < N; c++){
            if(used_col[c] || used_diag1[row + c] || used_diag2[row - c + N]) continue;
            used_col[c] = used_diag1[row + c] = used_diag2[row - c + N] = true;
            col[row] = c;
            solve(row + 1);
            used_col[c] = used_diag1[row + c] = used_diag2[row - c + N] = false;
        }
    };

    solve(0);
    return count;
}

// === 例題 ===
int main(){
    // N-Queen
    int N;
    cin >> N;
    cout << n_queen(N) << endl;
    return 0;
}
