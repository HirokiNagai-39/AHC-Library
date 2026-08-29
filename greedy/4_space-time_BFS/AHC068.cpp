#include <bits/stdc++.h>
using namespace std;

static constexpr int MAX_CELLS = 400;
static constexpr int INF = 1e9;
static constexpr int MAX_OUTPUT = 100000;

// 過去何手まで時空間探索するか。
// 全履歴を見るなら非常に大きくすればよいが、計算量が増える。
static constexpr int WINDOW = 100;

int N, S;

struct Operation
{
    char d;
    int r, c, h, w;

    bool operator==(const Operation &o) const
    {
        return d == o.d && r == o.r && c == o.c && h == o.h && w == o.w;
    }
};

// ------------------------------------------------------------
// あるカードが v にいるとき、op 適用後にどこへ行くか
// ------------------------------------------------------------
int apply_operation(const Operation &op, int v)
{
    int r = v / N;
    int c = v % N;

    // 長方形の外なら動かない
    if (r < op.r || r >= op.r + op.h ||
        c < op.c || c >= op.c + op.w)
    {
        return v;
    }

    int x = r - op.r;
    int y = c - op.c;

    if (op.d == 'V')
    {
        if (x < op.h / 2)
        {
            x += op.h / 2;
        }
        else
        {
            x -= op.h / 2;
        }
    }
    else
    {
        if (y < op.w / 2)
        {
            y += op.w / 2;
        }
        else
        {
            y -= op.w / 2;
        }
    }

    return (op.r + x) * N + (op.c + y);
}

// この解法が生成する操作は 1x2 / 2x1 のみ。
// その2マスを返す。
pair<int, int> endpoints(const Operation &op)
{
    int a = op.r * N + op.c;

    if (op.d == 'H')
    {
        return {a, a + 1};
    }
    else
    {
        return {a, a + N};
    }
}

// ------------------------------------------------------------
// 実際の盤面に隣接swapを適用
// ------------------------------------------------------------
void apply_board(
    const Operation &op,
    vector<int> &card_at,
    vector<int> &pos)
{
    auto [a, b] = endpoints(op);

    swap(card_at[a], card_at[b]);

    pos[card_at[a]] = a;
    pos[card_at[b]] = b;
}

// ------------------------------------------------------------
// target 番カードを target 番マスへ固定する
//
// state = (t, v)
//
// t : 既存操作列のどこまで消化したか
// v : targetカードの現在位置
//
// 既存操作:
//   (t,v) -> (t+1, apply(ops[t],v))
//   cost 0
//
// 新規操作:
//   (t,v) -> (t,nv)
//   cost 1
//
// を 0-1 BFS。
// ------------------------------------------------------------
bool insert_card(
    int target,
    vector<Operation> &ops,
    const vector<int> &initial_board,
    const vector<char> &fixed,
    const vector<vector<pair<int, int>>> &graph,
    const vector<Operation> &edge_ops)
{
    int T = ops.size();

    // 計算量削減のため、直近 WINDOW 手のみ過去改変する
    int L = max(0, T - WINDOW);
    int K = T - L;

    // ========================================================
    // 時刻 L の盤面を復元
    // ========================================================

    vector<int> card_at = initial_board;
    vector<int> pos(S);

    for (int v = 0; v < S; ++v)
    {
        pos[card_at[v]] = v;
    }

    for (int t = 0; t < L; ++t)
    {
        apply_board(ops[t], card_at, pos);
    }

    int start = pos[target];

    // ========================================================
    // blocked[t][v]
    //
    // 時刻 L+t において、
    // 既に固定済みのカードが v にいるか
    //
    // 新規操作はこのマスを触ってはいけない。
    // ========================================================

    vector<bitset<MAX_CELLS>> blocked(K + 1);

    for (int v = 0; v < S; ++v)
    {
        if (fixed[card_at[v]])
        {
            blocked[0].set(v);
        }
    }

    // 固定済みカードの「過去の軌跡」を作る
    for (int t = 0; t < K; ++t)
    {
        blocked[t + 1] = blocked[t];

        auto [a, b] = endpoints(ops[L + t]);

        bool x = blocked[t + 1].test(a);
        bool y = blocked[t + 1].test(b);

        blocked[t + 1].set(a, y);
        blocked[t + 1].set(b, x);
    }

    // ========================================================
    // 時空間 0-1 BFS
    // ========================================================

    int states = (K + 1) * S;

    vector<int> dist(states, INF);
    vector<int> parent(states, -1);

    // -1 : 既存操作
    // >=0: edge_ops の番号
    vector<int> parent_op(states, -2);

    vector<char> done(states, false);

    deque<int> dq;

    auto id = [&](int t, int v)
    {
        return t * S + v;
    };

    int start_state = id(0, start);
    int goal_state = id(K, target);

    dist[start_state] = 0;
    dq.push_front(start_state);

    while (!dq.empty())
    {
        int state = dq.front();
        dq.pop_front();

        if (done[state])
        {
            continue;
        }
        done[state] = true;

        int t = state / S;
        int v = state % S;
        int d = dist[state];

        if (state == goal_state)
        {
            break;
        }

        // ----------------------------------------------------
        // 既存操作をそのまま使う
        //
        // cost = 0
        // 時刻が +1
        // ----------------------------------------------------

        if (t < K)
        {
            int nv = apply_operation(ops[L + t], v);
            int next_state = id(t + 1, nv);

            if (dist[next_state] > d)
            {
                dist[next_state] = d;
                parent[next_state] = state;
                parent_op[next_state] = -1;

                dq.push_front(next_state);
            }
        }

        // ----------------------------------------------------
        // 新しい隣接swapを挿入
        //
        // cost = 1
        // 時刻は変化しない
        // ----------------------------------------------------

        for (auto [nv, op_id] : graph[v])
        {
            // nv に固定済みカードがいたら触れない
            if (blocked[t].test(nv))
            {
                continue;
            }

            int next_state = id(t, nv);

            if (dist[next_state] > d + 1)
            {
                dist[next_state] = d + 1;
                parent[next_state] = state;
                parent_op[next_state] = op_id;

                dq.push_back(next_state);
            }
        }
    }

    if (dist[goal_state] == INF)
    {
        return false;
    }

    if (T + dist[goal_state] > MAX_OUTPUT)
    {
        return false;
    }

    // ========================================================
    // 経路復元
    //
    // before[t]:
    // existing[L+t] の直前へ挿入する操作
    // ========================================================

    vector<pair<int, int>> inserted_rev;

    int cur = goal_state;

    while (cur != start_state)
    {
        int p = parent[cur];

        if (p == -1)
        {
            return false;
        }

        if (parent_op[cur] >= 0)
        {
            int t = p / S;

            inserted_rev.push_back({t,
                                    parent_op[cur]});
        }

        cur = p;
    }

    reverse(inserted_rev.begin(), inserted_rev.end());

    vector<vector<int>> before(K + 1);

    for (auto [t, op_id] : inserted_rev)
    {
        before[t].push_back(op_id);
    }

    // ========================================================
    // 新しい操作列を構築
    // ========================================================

    vector<Operation> next_ops;

    next_ops.reserve(
        T + dist[goal_state]);

    // 過去改変しないprefix
    for (int t = 0; t < L; ++t)
    {
        next_ops.push_back(ops[t]);
    }

    // 改変するsuffix
    for (int t = 0; t < K; ++t)
    {
        // 既存操作の直前に追加
        for (int op_id : before[t])
        {
            next_ops.push_back(edge_ops[op_id]);
        }

        // 元からあった操作
        next_ops.push_back(ops[L + t]);
    }

    // 全既存操作の後への追加
    for (int op_id : before[K])
    {
        next_ops.push_back(edge_ops[op_id]);
    }

    ops.swap(next_ops);

    return true;
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // ========================================================
    // Input
    // ========================================================

    cin >> N;

    S = N * N;

    vector<int> initial_board(S);

    for (int r = 0; r < N; ++r)
    {
        for (int c = 0; c < N; ++c)
        {
            cin >> initial_board[r * N + c];
        }
    }

    vector<string> V(N);
    vector<string> H(N - 1);

    for (auto &s : V)
    {
        cin >> s;
    }

    for (auto &s : H)
    {
        cin >> s;
    }

    // ========================================================
    // 壁のない隣接マスグラフ
    //
    // graph[v] = {移動先, OperationのID}
    // ========================================================

    vector<vector<pair<int, int>>> graph(S);
    vector<Operation> edge_ops;

    auto add_edge = [&](int a, int b, Operation op)
    {
        int op_id = edge_ops.size();

        edge_ops.push_back(op);

        graph[a].push_back({b,
                            op_id});

        graph[b].push_back({a,
                            op_id});
    };

    // --------------------------------------------------------
    // 左右の隣接交換
    //
    // H r c 1 2
    // --------------------------------------------------------

    for (int r = 0; r < N; ++r)
    {
        for (int c = 0; c + 1 < N; ++c)
        {
            if (V[r][c] == '0')
            {
                int a = r * N + c;
                int b = a + 1;

                add_edge(
                    a,
                    b,
                    Operation{
                        'H',
                        r,
                        c,
                        1,
                        2});
            }
        }
    }

    // --------------------------------------------------------
    // 上下の隣接交換
    //
    // V r c 2 1
    // --------------------------------------------------------

    for (int r = 0; r + 1 < N; ++r)
    {
        for (int c = 0; c < N; ++c)
        {
            if (H[r][c] == '0')
            {
                int a = r * N + c;
                int b = a + N;

                add_edge(
                    a,
                    b,
                    Operation{
                        'V',
                        r,
                        c,
                        2,
                        1});
            }
        }
    }

    // ========================================================
    // 固定順を作る
    //
    // 木の葉から順番に固定すると、
    // 未固定マスのグラフを連結に保てる。
    //
    // まずグラフ中心に近いrootを選ぶ。
    // ========================================================

    int root = 0;
    long long best_sum = (1LL << 60);

    vector<int> dist(S);
    queue<int> q;

    for (int s = 0; s < S; ++s)
    {
        fill(
            dist.begin(),
            dist.end(),
            -1);

        while (!q.empty())
        {
            q.pop();
        }

        dist[s] = 0;
        q.push(s);

        long long sum = 0;

        while (!q.empty())
        {
            int v = q.front();
            q.pop();

            sum += dist[v];

            for (auto [nv, op_id] : graph[v])
            {
                if (dist[nv] != -1)
                {
                    continue;
                }

                dist[nv] = dist[v] + 1;
                q.push(nv);
            }
        }

        if (sum < best_sum)
        {
            best_sum = sum;
            root = s;
        }
    }

    // ========================================================
    // rootからBFS木を作る
    // ========================================================

    vector<int> depth(S, -1);

    depth[root] = 0;
    q.push(root);

    while (!q.empty())
    {
        int v = q.front();
        q.pop();

        for (auto [nv, op_id] : graph[v])
        {
            if (depth[nv] != -1)
            {
                continue;
            }

            depth[nv] = depth[v] + 1;
            q.push(nv);
        }
    }

    // 深い頂点から固定する。
    // BFS木における葉刈りになる。
    vector<int> order;

    for (int v = 0; v < S; ++v)
    {
        if (v != root)
        {
            order.push_back(v);
        }
    }

    stable_sort(
        order.begin(),
        order.end(),
        [&](int a, int b)
        {
            return depth[a] > depth[b];
        });

    // ========================================================
    // 過去改変貪欲
    // ========================================================

    vector<char> fixed(S, false);

    vector<Operation> ops;
    ops.reserve(20000);

    for (int target : order)
    {
        bool ok = insert_card(
            target,
            ops,
            initial_board,
            fixed,
            graph,
            edge_ops);

        if (!ok)
        {
            break;
        }

        fixed[target] = true;
    }

    // ========================================================
    // 完全に同じswapが連続していたら
    //
    // A A = identity
    //
    // なので消す。
    // ========================================================

    vector<Operation> reduced;

    reduced.reserve(ops.size());

    for (const auto &op : ops)
    {
        if (
            !reduced.empty() && reduced.back() == op)
        {
            reduced.pop_back();
        }
        else
        {
            reduced.push_back(op);
        }
    }

    ops.swap(reduced);

    if ((int)ops.size() > MAX_OUTPUT)
    {
        ops.resize(MAX_OUTPUT);
    }

    // AHC068は操作数Tそのものを先頭には出力せず、
    // 操作を1行ずつ出力する。
    for (const auto &op : ops)
    {
        cout
            << op.d << ' '
            << op.r << ' '
            << op.c << ' '
            << op.h << ' '
            << op.w << '\n';
    }
}