#include <bits/stdc++.h>
using namespace std;

const int INF = 1e9;

struct Operation
{
    int a, b;
};

// 状態 v に操作 op を適用した後の状態
int apply(const Operation &op, int v)
{
    if (v == op.a)
        return op.b;
    if (v == op.b)
        return op.a;
    return v;
}

struct Prev
{
    int t = -1;
    int v = -1;

    Operation op{-1, -1};

    // true  : 新しい操作を挿入した
    // false : 既存操作をそのまま使った
    bool inserted = false;
};

int main()
{
    int V, T;
    cin >> V >> T;

    // existing[t]:
    // 元の操作列の t 番目の操作
    vector<Operation> existing(T);

    for (auto &op : existing)
    {
        cin >> op.a >> op.b;
    }

    // insertable[v]:
    // 状態 v にいるとき挿入候補となる操作
    vector<vector<Operation>> insertable(V);

    int M;
    cin >> M;

    for (int i = 0; i < M; ++i)
    {
        Operation op;
        cin >> op.a >> op.b;

        // この例では a,b のどちらにいても意味がある操作
        insertable[op.a].push_back(op);
        insertable[op.b].push_back(op);
    }

    int start, goal;
    cin >> start >> goal;

    vector<vector<int>> dist(T + 1, vector<int>(V, INF));
    vector<vector<Prev>> pre(T + 1, vector<Prev>(V));

    deque<pair<int, int>> dq;

    dist[0][start] = 0;
    dq.push_front({0, start});

    while (!dq.empty())
    {
        auto [t, v] = dq.front();
        dq.pop_front();

        int d = dist[t][v];

        // --------------------------------
        // 既存操作を使う
        //
        // (t, v) -> (t+1, nv)
        // cost = 0
        // --------------------------------
        if (t < T)
        {
            int nv = apply(existing[t], v);

            if (dist[t + 1][nv] > d)
            {
                dist[t + 1][nv] = d;

                pre[t + 1][nv] = {
                    t,
                    v,
                    existing[t],
                    false};

                dq.push_front({t + 1, nv});
            }
        }

        // --------------------------------
        // 新しい操作を挿入
        //
        // (t, v) -> (t, nv)
        // cost = 1
        // --------------------------------
        for (const auto &op : insertable[v])
        {
            int nv = apply(op, v);

            if (dist[t][nv] > d + 1)
            {
                dist[t][nv] = d + 1;

                pre[t][nv] = {
                    t,
                    v,
                    op,
                    true};

                dq.push_back({t, nv});
            }
        }
    }

    if (dist[T][goal] == INF)
    {
        cout << "unreachable\n";
        return 0;
    }

    cout << "additional operations = "
         << dist[T][goal] << '\n';

    // ============================================================
    // 経路復元
    // ============================================================

    struct Step
    {
        int time;
        Operation op;
        bool inserted;
    };

    vector<Step> path;

    int t = T;
    int v = goal;

    while (!(t == 0 && v == start))
    {
        Prev p = pre[t][v];

        if (p.t == -1)
        {
            cout << "restore failed\n";
            return 0;
        }

        path.push_back({p.t,
                        p.op,
                        p.inserted});

        t = p.t;
        v = p.v;
    }

    reverse(path.begin(), path.end());

    // ============================================================
    // 復元結果
    // ============================================================

    for (auto step : path)
    {
        if (step.inserted)
        {
            cout << "insert : "
                 << step.op.a << ' '
                 << step.op.b
                 << " before existing["
                 << step.time << "]\n";
        }
        else
        {
            cout << "existing[" << step.time << "] : "
                 << step.op.a << ' '
                 << step.op.b << '\n';
        }
    }
}