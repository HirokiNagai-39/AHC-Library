#include <bits/stdc++.h>
using namespace std;

struct Operation
{
    char d;
    int r, c, h, w;
};

static inline bool sameOp(const Operation &a, const Operation &b)
{
    return a.d == b.d && a.r == b.r && a.c == b.c && a.h == b.h && a.w == b.w;
}

static inline uint64_t splitMix(uint64_t x)
{
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

struct Solver
{
    static constexpr int N = 20;
    static constexpr int M = 400;
    static constexpr int MAXCAND = 13;
    static constexpr int jitterChoices = 2;
    static constexpr int jitterPermille = 125;
    static constexpr int midShiftGap = 2;
    static constexpr int deepBridgeLimit = 24;
    static constexpr int meetSlotLimit = 4;
    static constexpr int jumpEvalLimit = 72;
    static constexpr int jumpEarlyEvalLimit = 8;

    struct Meet
    {
        int t;
        int stamp;
        short u, v;
        int eu, ev;
        int rec;
    };

    struct TimedOperation
    {
        int t;
        int ord;
        Operation op;
    };

    struct InsertionMark
    {
        int order;
        int t;
    };

    struct Candidate
    {
        long long score;
        int area;
        Operation op;
    };

    vector<int> initA;
    vector<string> Vs, Hs;

    int wallR[N][N - 1];
    int wallD[N - 1][N];

    vector<int> adj[M];
    vector<pair<int, int>> edges;

    short distAll[M][M];

    int prefV[N + 1][N + 1];
    int prefH[N + 1][N + 1];

    int a[M];
    int pos[M];
    bool alive[M];

    int epoch[M];
    vector<Meet> meetLast;
    vector<Meet> meetFirst;
    vector<Meet> meetMidLate;
    vector<Meet> meetMidEarly;
    vector<vector<InsertionMark>> insertionLog;
    int runStamp = 0;

    vector<Operation> baseOps;
    vector<TimedOperation> retroOps;
    int retroCount = 0;

    bool extensionsEnabled = false;
    bool deepChainsEnabled = false;
    bool jitterEnabled = false;
    bool lookaheadEnabled = false;
    long long deepChainAcceptedRun = 0;
    long long deepChainSavedRun = 0;
    int firstCand = 6;
    bool aborted = false;
    chrono::steady_clock::time_point clockStart;
    double abortLimitMs = 1e18;
    uint64_t randomState = 0x2545F4914F6CDD1DULL;

    int prefA[N + 1][N + 1];

    int gainPref[N + 1][N + 1];

    int leafCount = 0;

    int rowOf[M];
    int colOf[M];
    int neighborCount[M];
    int neighborCell[M][4];
    uint64_t cellAdjacency[M][7];

    int colRunStart[N][N];
    int colRunEnd[N][N];
    int rowRunStart[N][N];
    int rowRunEnd[N][N];

    int touchedCellA[832];
    int touchedCellB[832];
    int capturedX[832];
    int capturedY[832];
    int capturedEU[832];
    int capturedEV[832];
    int touchedCount = 0;
    int capturedRec = 0;
    uint64_t headFresh[(M * M + 63) / 64];

    int searchPrev[M];
    int searchMark[M];
    int searchMarkCur = 0;
    int searchQueue[M];
    int pathCells[M];

    unordered_map<int, int> treeCacheIndex;
    vector<vector<int>> treeParentCache;
    vector<vector<int>> treeDepthCache;
    vector<vector<int>> treeChildCache;
    vector<vector<int>> treeLeafListCache;

    int childCount[M];
    int leafCells[M];
    int leafSlot[M];

    bool anchorFoundScratch[M];
    int anchorTimeScratch[M];
    Meet anchorMeetScratch[M];
    char finalKnownScratch[M];
    int finalCountScratch[M];
    int finalEarliestScratch[M];
    Meet finalMeetScratch[M][meetSlotLimit];

    vector<array<short, M>> histPos;
    vector<array<short, M>> histCard;
    vector<int> histRec;
    int histLen = 0;
    int involvedStamp[M];
    int involvedStampCounter = 0;
    long long jumpRetroAcceptedRun = 0;

    Solver(const vector<int> &initialCardInput, const vector<string> &verticalWallInput, const vector<string> &horizontalWallInput)
        : initA(initialCardInput), Vs(verticalWallInput), Hs(horizontalWallInput),
          meetLast(M * M), meetFirst(M * M), meetMidLate(M * M), meetMidEarly(M * M), insertionLog(M)
    {
        memset(wallR, 0, sizeof(wallR));
        memset(wallD, 0, sizeof(wallD));
        memset(gainPref, 0, sizeof(gainPref));
        memset(prefA, 0, sizeof(prefA));
        memset(cellAdjacency, 0, sizeof(cellAdjacency));
        memset(searchMark, 0, sizeof(searchMark));
        memset(searchPrev, 0, sizeof(searchPrev));
        memset(involvedStamp, 0, sizeof(involvedStamp));

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N - 1; j++)
            {
                wallR[i][j] = (Vs[i][j] == '1');
            }
        }

        for (int i = 0; i < N - 1; i++)
        {
            for (int j = 0; j < N; j++)
            {
                wallD[i][j] = (Hs[i][j] == '1');
            }
        }

        for (int v = 0; v < M; v++)
        {
            rowOf[v] = v / N;
            colOf[v] = v % N;
        }

        buildGraph();

        for (int v = 0; v < M; v++)
        {
            neighborCount[v] = (int)adj[v].size();

            for (int k = 0; k < neighborCount[v]; k++)
            {
                neighborCell[v][k] = adj[v][k];
            }
        }

        for (auto [u, v] : edges)
        {
            cellAdjacency[u][v >> 6] |= 1ULL << (v & 63);
            cellAdjacency[v][u >> 6] |= 1ULL << (u & 63);
        }

        buildStaticPrefixes();
        allPairsBFS();
    }

    inline int id(int r, int c) const
    {
        return r * N + c;
    }

    inline int rr(int v) const
    {
        return rowOf[v];
    }

    inline int cc(int v) const
    {
        return colOf[v];
    }

    double elapsedMs() const
    {
        return chrono::duration<double, milli>(chrono::steady_clock::now() - clockStart).count();
    }

    void seedRandom(uint64_t seed)
    {
        randomState = seed ? seed : 0x2545F4914F6CDD1DULL;
    }

    inline uint64_t nextRandom()
    {
        randomState ^= randomState << 13;
        randomState ^= randomState >> 7;
        randomState ^= randomState << 17;
        return randomState;
    }

    inline int randomBelow(int n)
    {
        return (int)(nextRandom() % (uint64_t)n);
    }

    inline bool jitterRoll()
    {
        return jitterEnabled && (int)(nextRandom() % 1000) < jitterPermille;
    }

    void buildGraph()
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                int v = id(i, j);

                if (i + 1 < N && !wallD[i][j])
                {
                    int u = id(i + 1, j);
                    adj[v].push_back(u);
                    adj[u].push_back(v);
                    edges.push_back({v, u});
                }

                if (j + 1 < N && !wallR[i][j])
                {
                    int u = id(i, j + 1);
                    adj[v].push_back(u);
                    adj[u].push_back(v);
                    edges.push_back({v, u});
                }
            }
        }
    }

    void buildStaticPrefixes()
    {
        memset(prefV, 0, sizeof(prefV));
        for (int r = 0; r < N; r++)
        {
            for (int c = 0; c < N - 1; c++)
            {
                prefV[r + 1][c + 1] =
                    prefV[r + 1][c] + prefV[r][c + 1] - prefV[r][c] + wallR[r][c];
            }
        }

        memset(prefH, 0, sizeof(prefH));
        for (int r = 0; r < N - 1; r++)
        {
            for (int c = 0; c < N; c++)
            {
                prefH[r + 1][c + 1] =
                    prefH[r + 1][c] + prefH[r][c + 1] - prefH[r][c] + wallD[r][c];
            }
        }
    }

    void allPairsBFS()
    {
        for (int s = 0; s < M; s++)
        {
            short *drow = distAll[s];

            for (int i = 0; i < M; i++)
            {
                drow[i] = 30000;
            }

            drow[s] = 0;
            searchQueue[0] = s;
            int qh = 0, qt = 1;

            while (qh < qt)
            {
                int v = searchQueue[qh++];
                short nd = (short)(drow[v] + 1);

                for (int u : adj[v])
                {
                    if (drow[u] > nd)
                    {
                        drow[u] = nd;
                        searchQueue[qt++] = u;
                    }
                }
            }
        }
    }

    inline int sumPref(const int pref[N + 1][N + 1], int r, int c, int h, int w) const
    {
        if (h <= 0 || w <= 0)
            return 0;
        return pref[r + h][c + w] - pref[r][c + w] - pref[r + h][c] + pref[r][c];
    }

    bool wallFree(int r, int c, int h, int w) const
    {
        if (r < 0 || c < 0 || h <= 0 || w <= 0 || r + h > N || c + w > N)
            return false;
        if (w >= 2 && sumPref(prefV, r, c, h, w - 1) != 0)
            return false;
        if (h >= 2 && sumPref(prefH, r, c, h - 1, w) != 0)
            return false;
        return true;
    }

    inline bool cellsAdjacent(int p, int q) const
    {
        return (cellAdjacency[p][q >> 6] >> (q & 63)) & 1ULL;
    }

    void rebuildPrefA()
    {
        for (int r = 0; r < N; r++)
        {
            const bool *aliveRow = alive + r * N;
            int *cur = prefA[r + 1];
            const int *above = prefA[r];
            int rowSum = 0;

            for (int c = 0; c < N; c++)
            {
                rowSum += aliveRow[c] ? 1 : 0;
                cur[c + 1] = above[c + 1] + rowSum;
            }
        }
    }

    void rebuildColumnRuns(int c)
    {
        int r = 0;

        while (r < N)
        {
            if (!alive[id(r, c)])
            {
                colRunStart[c][r] = r;
                colRunEnd[c][r] = r - 1;
                r++;
                continue;
            }

            int start = r;

            while (r + 1 < N && alive[id(r + 1, c)] && !wallD[r][c])
            {
                r++;
            }

            for (int i = start; i <= r; i++)
            {
                colRunStart[c][i] = start;
                colRunEnd[c][i] = r;
            }

            r++;
        }
    }

    void rebuildRowRuns(int r)
    {
        int c = 0;

        while (c < N)
        {
            if (!alive[id(r, c)])
            {
                rowRunStart[r][c] = c;
                rowRunEnd[r][c] = c - 1;
                c++;
                continue;
            }

            int start = c;

            while (c + 1 < N && alive[id(r, c + 1)] && !wallR[r][c])
            {
                c++;
            }

            for (int i = start; i <= c; i++)
            {
                rowRunStart[r][i] = start;
                rowRunEnd[r][i] = c;
            }

            c++;
        }
    }

    void rebuildActiveTables()
    {
        rebuildPrefA();

        for (int c = 0; c < N; c++)
        {
            rebuildColumnRuns(c);
        }

        for (int r = 0; r < N; r++)
        {
            rebuildRowRuns(r);
        }
    }

    void applyCellDeath(int cell)
    {
        rebuildPrefA();
        rebuildColumnRuns(colOf[cell]);
        rebuildRowRuns(rowOf[cell]);
    }

    void initRun(bool useExtensions)
    {
        extensionsEnabled = useExtensions;
        deepChainsEnabled = useExtensions;
        deepChainAcceptedRun = 0;
        deepChainSavedRun = 0;
        jumpRetroAcceptedRun = 0;
        histLen = 0;
        aborted = false;

        memcpy(a, initA.data(), sizeof(a));

        for (int i = 0; i < M; i++)
        {
            pos[a[i]] = i;
        }

        fill(alive, alive + M, true);
        fill(epoch, epoch + M, 0);

        runStamp++;

        for (auto &log : insertionLog)
        {
            log.clear();
        }

        baseOps.clear();
        retroOps.clear();
        retroCount = 0;

        rebuildActiveTables();

        if (extensionsEnabled)
        {
            memset(headFresh, 0, sizeof(headFresh));

            for (auto [u, v] : edges)
            {
                int x = a[u];
                int y = a[v];
                int lo = min(x, y);
                int hi = max(x, y);
                int idx = lo * M + hi;

                meetFirst[idx] = Meet{0, runStamp, (short)u, (short)v, epoch[x], epoch[y], retroCount};
                headFresh[idx >> 6] |= 1ULL << (idx & 63);
            }
        }
    }

    Operation edgeOperation(int u, int v) const
    {
        int ur = rr(u), uc = cc(u);
        int vr = rr(v), vc = cc(v);

        if (uc == vc)
        {
            return Operation{'V', min(ur, vr), uc, 2, 1};
        }
        else
        {
            return Operation{'H', ur, min(uc, vc), 1, 2};
        }
    }

    bool meetAccurate(const Meet &m, int x, int y) const
    {
        if (m.t < 0 || m.stamp != runStamp)
            return false;

        for (const auto &mark : insertionLog[x])
        {
            if (mark.order >= m.rec && mark.t <= m.t)
                return false;
        }

        for (const auto &mark : insertionLog[y])
        {
            if (mark.order >= m.rec && mark.t <= m.t)
                return false;
        }

        return true;
    }

    inline Meet currentMeet(int x, int y) const
    {
        return Meet{(int)baseOps.size(), runStamp, (short)pos[x], (short)pos[y], epoch[x], epoch[y], retroCount};
    }

    inline bool effectiveLastExists(int x, int y) const
    {
        if (cellsAdjacent(pos[x], pos[y]))
            return true;

        const Meet &m = meetLast[x * M + y];
        return m.t >= 0 && m.stamp == runStamp;
    }

    bool bestMeetBelowInto(int cardA, int cardB, int bound, Meet &out) const
    {
        if (cellsAdjacent(pos[cardA], pos[cardB]))
        {
            if ((int)baseOps.size() < bound)
            {
                out = currentMeet(cardA, cardB);
                return true;
            }
        }
        else
        {
            const Meet &last = meetLast[cardA * M + cardB];

            if (last.stamp != runStamp || last.t < 0)
                return false;

            if (last.t < bound && meetAccurate(last, cardA, cardB))
            {
                out = last;
                return true;
            }
        }

        int lo = min(cardA, cardB);
        int hi = max(cardA, cardB);
        int slot = lo * M + hi;

        const Meet *best = nullptr;

        const Meet &lateMid = meetMidLate[slot];
        if (lateMid.t < bound && meetAccurate(lateMid, cardA, cardB))
            best = &lateMid;

        const Meet &earlyMid = meetMidEarly[slot];
        if (earlyMid.t < bound && (!best || earlyMid.t > best->t) && meetAccurate(earlyMid, cardA, cardB))
            best = &earlyMid;

        const Meet &head = meetFirst[slot];
        if (head.t < bound && (!best || head.t > best->t) && meetAccurate(head, cardA, cardB))
            best = &head;

        if (!best)
            return false;

        out = *best;
        return true;
    }

    void collectFinalMeets(int mover, int bridge)
    {
        int count = 0;
        Meet *slots = finalMeetScratch[bridge];

        if (cellsAdjacent(pos[mover], pos[bridge]))
        {
            slots[count++] = currentMeet(mover, bridge);
        }
        else
        {
            const Meet &last = meetLast[mover * M + bridge];

            if (meetAccurate(last, mover, bridge))
                slots[count++] = last;
        }

        int lo = min(mover, bridge);
        int hi = max(mover, bridge);
        int slot = lo * M + hi;

        const Meet &lateMid = meetMidLate[slot];
        if (meetAccurate(lateMid, mover, bridge))
            slots[count++] = lateMid;

        const Meet &earlyMid = meetMidEarly[slot];
        if (meetAccurate(earlyMid, mover, bridge))
            slots[count++] = earlyMid;

        const Meet &head = meetFirst[slot];
        if (meetAccurate(head, mover, bridge))
            slots[count++] = head;

        sort(slots, slots + count, [](const Meet &p, const Meet &q)
             { return p.t < q.t; });

        finalCountScratch[bridge] = count;
        finalEarliestScratch[bridge] = count ? slots[0].t : INT_MAX;
    }

    bool epochValid(int x, int y) const
    {
        if (cellsAdjacent(pos[x], pos[y]))
            return true;

        const Meet &m = meetLast[x * M + y];
        return m.stamp == runStamp && m.t >= 0 && m.eu == epoch[x] && m.ev == epoch[y];
    }

    inline void captureEdge(int p, int q)
    {
        int x = a[p];
        int y = a[q];

        touchedCellA[touchedCount] = p;
        touchedCellB[touchedCount] = q;
        capturedX[touchedCount] = x;
        capturedY[touchedCount] = y;
        capturedEU[touchedCount] = epoch[x];
        capturedEV[touchedCount] = epoch[y];
        touchedCount++;
    }

    void captureRectContacts(const Operation &op)
    {
        touchedCount = 0;
        capturedRec = retroCount;

        int rowEnd = op.r + op.h;
        int colEnd = op.c + op.w;

        for (int r = op.r; r < rowEnd; r++)
        {
            for (int c = op.c; c < colEnd; c++)
            {
                int p = id(r, c);

                if (r + 1 < N && !wallD[r][c])
                    captureEdge(p, p + N);

                if (c + 1 < N && !wallR[r][c])
                    captureEdge(p, p + 1);
            }

            if (op.c > 0 && !wallR[r][op.c - 1])
                captureEdge(id(r, op.c - 1), id(r, op.c));
        }

        if (op.r > 0)
        {
            for (int c = op.c; c < colEnd; c++)
            {
                if (!wallD[op.r - 1][c])
                    captureEdge(id(op.r - 1, c), id(op.r, c));
            }
        }
    }

    void captureSwapContacts(int p, int q)
    {
        touchedCount = 0;
        capturedRec = retroCount;

        int cnt = neighborCount[p];

        for (int k = 0; k < cnt; k++)
        {
            int nb = neighborCell[p][k];
            int x = a[p];
            int y = a[nb];

            touchedCellA[touchedCount] = p;
            touchedCellB[touchedCount] = nb;
            capturedX[touchedCount] = x;
            capturedY[touchedCount] = y;
            capturedEU[touchedCount] = epoch[x];
            capturedEV[touchedCount] = epoch[y];
            touchedCount++;
        }

        cnt = neighborCount[q];

        for (int k = 0; k < cnt; k++)
        {
            int nb = neighborCell[q][k];

            if (nb == p)
                continue;

            int x = a[q];
            int y = a[nb];

            touchedCellA[touchedCount] = q;
            touchedCellB[touchedCount] = nb;
            capturedX[touchedCount] = x;
            capturedY[touchedCount] = y;
            capturedEU[touchedCount] = epoch[x];
            capturedEV[touchedCount] = epoch[y];
            touchedCount++;
        }
    }

    void writeBreaks(int t)
    {
        for (int i = 0; i < touchedCount; i++)
        {
            int x = capturedX[i];
            int y = capturedY[i];

            if (cellsAdjacent(pos[x], pos[y]))
                continue;

            short u = (short)touchedCellA[i];
            short v = (short)touchedCellB[i];

            meetLast[x * M + y] = Meet{t, runStamp, u, v, capturedEU[i], capturedEV[i], capturedRec};
            meetLast[y * M + x] = Meet{t, runStamp, u, v, capturedEV[i], capturedEU[i], capturedRec};
        }
    }

    void refreshHeads(int t)
    {
        if (!extensionsEnabled)
            return;

        for (int i = 0; i < touchedCount; i++)
        {
            int u = touchedCellA[i];
            int v = touchedCellB[i];
            int x = a[u];
            int y = a[v];
            int lo = min(x, y);
            int hi = max(x, y);
            int idx = lo * M + hi;
            uint64_t bit = 1ULL << (idx & 63);
            uint64_t &word = headFresh[idx >> 6];

            if (word & bit)
            {
                if (insertionLog[x].empty() && insertionLog[y].empty())
                    continue;

                Meet &head = meetFirst[idx];

                if (!meetAccurate(head, x, y))
                {
                    head = Meet{t, runStamp, (short)u, (short)v, epoch[x], epoch[y], retroCount};
                }
            }
            else
            {
                word |= bit;
                meetFirst[idx] = Meet{t, runStamp, (short)u, (short)v, epoch[x], epoch[y], retroCount};
            }
        }
    }

    inline int rectSourceCell(const Operation &op, int cell) const
    {
        int r = rowOf[cell];
        int c = colOf[cell];

        if (r < op.r || r >= op.r + op.h || c < op.c || c >= op.c + op.w)
            return cell;

        if (op.d == 'V')
        {
            int half = op.h / 2;
            return r < op.r + half ? cell + half * N : cell - half * N;
        }

        int half = op.w / 2;
        return c < op.c + half ? cell + half : cell - half;
    }

    inline int swapSourceCell(int p, int q, int cell) const
    {
        if (cell == p)
            return q;
        if (cell == q)
            return p;
        return cell;
    }

    inline void shiftMidPair(int xCard, int yCard, int t)
    {
        const Meet &previous = meetLast[xCard * M + yCard];

        if (previous.stamp == runStamp && previous.t >= 0 && t >= previous.t + midShiftGap && meetAccurate(previous, xCard, yCard))
        {
            int lo = min(xCard, yCard);
            int hi = max(xCard, yCard);

            meetMidEarly[lo * M + hi] = meetMidLate[lo * M + hi];
            meetMidLate[lo * M + hi] = previous;
        }
    }

    void shiftMidsAfterRect(const Operation &op, int t)
    {
        if (!extensionsEnabled)
            return;

        for (int i = 0; i < touchedCount; i++)
        {
            int u = touchedCellA[i];
            int v = touchedCellB[i];
            int sourceU = rectSourceCell(op, u);
            int sourceV = rectSourceCell(op, v);

            if (cellsAdjacent(sourceU, sourceV))
                continue;

            shiftMidPair(a[u], a[v], t);
        }
    }

    void shiftMidsAfterSwap(int p, int q, int t)
    {
        if (!extensionsEnabled)
            return;

        for (int i = 0; i < touchedCount; i++)
        {
            int u = touchedCellA[i];
            int v = touchedCellB[i];
            int sourceU = swapSourceCell(p, q, u);
            int sourceV = swapSourceCell(p, q, v);

            if (cellsAdjacent(sourceU, sourceV))
                continue;

            shiftMidPair(a[u], a[v], t);
        }
    }

    bool validMeet(int x, int y) const
    {
        if (x == y)
            return false;
        if (!alive[x] || !alive[y])
            return false;

        if (!extensionsEnabled)
            return epochValid(x, y);

        Meet found;
        return bestMeetBelowInto(x, y, INT_MAX, found);
    }

    bool pickMeetInto(int x, int y, Meet &out) const
    {
        if (!extensionsEnabled)
        {
            if (cellsAdjacent(pos[x], pos[y]))
            {
                out = currentMeet(x, y);
                return true;
            }

            const Meet &m = meetLast[x * M + y];

            if (m.stamp == runStamp && m.t >= 0 && m.eu == epoch[x] && m.ev == epoch[y])
            {
                out = m;
                return true;
            }

            return false;
        }

        return bestMeetBelowInto(x, y, INT_MAX, out);
    }

    void applyRetroInsertion(int x, int y, Meet m)
    {
        int px = pos[x];
        int py = pos[y];

        captureSwapContacts(px, py);

        retroOps.push_back(TimedOperation{m.t, retroCount, edgeOperation(m.u, m.v)});

        insertionLog[x].push_back(InsertionMark{retroCount, m.t});
        insertionLog[y].push_back(InsertionMark{retroCount, m.t});
        retroCount++;

        epoch[x]++;
        epoch[y]++;

        swap(a[px], a[py]);
        swap(pos[x], pos[y]);

        writeBreaks((int)baseOps.size());
        refreshHeads((int)baseOps.size());
        shiftMidsAfterSwap(px, py, (int)baseOps.size());
    }

    bool retroSwapIfMet(int x, int y)
    {
        if (x == y)
            return false;
        if (!alive[x] || !alive[y])
            return false;

        Meet m;

        if (!pickMeetInto(x, y, m))
            return false;

        applyRetroInsertion(x, y, m);
        return true;
    }

    bool tryPastFix(int target)
    {
        if (a[target] == target)
            return true;

        int x = target;
        int y = a[target];

        if (!alive[x] || !alive[y])
            return false;
        if (!retroSwapIfMet(x, y))
            return false;

        return a[target] == target;
    }

    void snapshotHistory()
    {
        if ((int)histPos.size() <= histLen)
        {
            histPos.emplace_back();
            histCard.emplace_back();
            histRec.push_back(0);
        }

        short *posSnap = histPos[histLen].data();
        short *cardSnap = histCard[histLen].data();

        for (int i = 0; i < M; i++)
        {
            posSnap[i] = (short)pos[i];
            cardSnap[i] = (short)a[i];
        }

        histRec[histLen] = retroCount;
        histLen++;
    }

    bool snapshotCardValid(int card, int tau) const
    {
        int recBound = histRec[tau];

        for (const auto &mark : insertionLog[card])
        {
            if (mark.order >= recBound && mark.t <= tau)
                return false;
        }

        return true;
    }

    void evaluateThinColumn(int tau, int col, int lowRow, int span, int x, int y, long long budget, long long &bestSide, Operation &bestOp, int &bestTau)
    {
        const short *cardSnap = histCard[tau].data();

        for (int k = 0; k < span; k++)
        {
            int top = lowRow - k;

            if (top < 0)
                break;
            if (top + 2 * span > N)
                continue;
            if (sumPref(prefH, top, col, 2 * span - 1, 1) != 0)
                continue;

            long long side = 0;
            bool usable = true;
            bool coversTarget = false;

            for (int j = 0; j < span; j++)
            {
                int upCell = id(top + j, col);
                int downCell = upCell + span * N;
                int upCard = cardSnap[upCell];
                int downCard = cardSnap[downCell];

                if ((upCard == x && downCard == y) || (upCard == y && downCard == x))
                {
                    coversTarget = true;
                    continue;
                }

                if (!alive[upCard] || !alive[downCard] ||
                    !snapshotCardValid(upCard, tau) || !snapshotCardValid(downCard, tau))
                {
                    usable = false;
                    break;
                }

                int curUp = pos[upCard];
                int curDown = pos[downCard];

                side += distAll[curDown][upCard] + distAll[curUp][downCard] -
                        distAll[curUp][upCard] - distAll[curDown][downCard];
            }

            if (!usable || !coversTarget)
                continue;
            if (side > budget || side >= bestSide)
                continue;

            bestSide = side;
            bestOp = Operation{'V', top, col, 2 * span, 1};
            bestTau = tau;
        }
    }

    void evaluateThinRow(int tau, int row, int lowCol, int span, int x, int y, long long budget, long long &bestSide, Operation &bestOp, int &bestTau)
    {
        const short *cardSnap = histCard[tau].data();

        for (int k = 0; k < span; k++)
        {
            int left = lowCol - k;

            if (left < 0)
                break;
            if (left + 2 * span > N)
                continue;
            if (sumPref(prefV, row, left, 1, 2 * span - 1) != 0)
                continue;

            long long side = 0;
            bool usable = true;
            bool coversTarget = false;

            for (int j = 0; j < span; j++)
            {
                int leftCell = id(row, left + j);
                int rightCell = leftCell + span;
                int leftCard = cardSnap[leftCell];
                int rightCard = cardSnap[rightCell];

                if ((leftCard == x && rightCard == y) || (leftCard == y && rightCard == x))
                {
                    coversTarget = true;
                    continue;
                }

                if (!alive[leftCard] || !alive[rightCard] ||
                    !snapshotCardValid(leftCard, tau) || !snapshotCardValid(rightCard, tau))
                {
                    usable = false;
                    break;
                }

                int curLeft = pos[leftCard];
                int curRight = pos[rightCard];

                side += distAll[curRight][leftCard] + distAll[curLeft][rightCard] -
                        distAll[curLeft][leftCard] - distAll[curRight][rightCard];
            }

            if (!usable || !coversTarget)
                continue;
            if (side > budget || side >= bestSide)
                continue;

            bestSide = side;
            bestOp = Operation{'H', row, left, 1, 2 * span};
            bestTau = tau;
        }
    }

    bool applyRetroJump(const Operation &op, int tau)
    {
        const short *cardSnap = histCard[tau].data();
        int involvedCards[N];
        int pairCount = 0;

        if (op.d == 'V')
        {
            int half = op.h / 2;

            for (int k = 0; k < half; k++)
            {
                int upCell = id(op.r + k, op.c);
                involvedCards[2 * pairCount] = cardSnap[upCell];
                involvedCards[2 * pairCount + 1] = cardSnap[upCell + half * N];
                pairCount++;
            }
        }
        else
        {
            int half = op.w / 2;

            for (int k = 0; k < half; k++)
            {
                int leftCell = id(op.r, op.c + k);
                involvedCards[2 * pairCount] = cardSnap[leftCell];
                involvedCards[2 * pairCount + 1] = cardSnap[leftCell + half];
                pairCount++;
            }
        }

        for (int i = 0; i < 2 * pairCount; i++)
        {
            int card = involvedCards[i];

            if (!alive[card] || !snapshotCardValid(card, tau))
                return false;
        }

        touchedCount = 0;
        capturedRec = retroCount;
        involvedStampCounter++;

        for (int i = 0; i < 2 * pairCount; i++)
        {
            involvedStamp[pos[involvedCards[i]]] = involvedStampCounter;
        }

        for (int i = 0; i < 2 * pairCount; i++)
        {
            int p = pos[involvedCards[i]];
            int cnt = neighborCount[p];

            for (int k = 0; k < cnt; k++)
            {
                int nb = neighborCell[p][k];

                if (involvedStamp[nb] == involvedStampCounter && nb < p)
                    continue;

                captureEdge(p, nb);
            }
        }

        retroOps.push_back(TimedOperation{tau, retroCount, op});

        for (int i = 0; i < 2 * pairCount; i++)
        {
            int card = involvedCards[i];
            insertionLog[card].push_back(InsertionMark{retroCount, tau});
            epoch[card]++;
        }

        retroCount++;

        for (int k = 0; k < pairCount; k++)
        {
            int first = involvedCards[2 * k];
            int second = involvedCards[2 * k + 1];
            int firstPos = pos[first];
            int secondPos = pos[second];

            swap(a[firstPos], a[secondPos]);
            pos[first] = secondPos;
            pos[second] = firstPos;
        }

        writeBreaks((int)baseOps.size());
        refreshHeads((int)baseOps.size());

        return true;
    }

    bool tryPastMacroFix(int target, int forwardCost)
    {
        if (!extensionsEnabled || histLen == 0)
            return false;
        if (a[target] == target)
            return true;

        int x = target;
        int y = a[target];

        if (!alive[x] || !alive[y])
            return false;

        if (elapsedMs() > abortLimitMs)
        {
            aborted = true;
            return false;
        }

        long long budget = (long long)distAll[pos[x]][x] + 2LL * (forwardCost - 1);
        long long bestSide = LLONG_MAX;
        Operation bestOp{'V', 0, 0, 2, 1};
        int bestTau = -1;
        int evaluated = 0;

        for (int tau = histLen - 1; tau >= 0; tau--)
        {
            const short *posSnap = histPos[tau].data();
            int px = posSnap[x];
            int py = posSnap[y];

            if (colOf[px] == colOf[py])
            {
                int span = abs(rowOf[px] - rowOf[py]);
                int lowRow = min(rowOf[px], rowOf[py]);

                if (2 * span > N)
                    continue;
                if (sumPref(prefH, lowRow, colOf[px], span, 1) != 0)
                    continue;
                if (!snapshotCardValid(x, tau) || !snapshotCardValid(y, tau))
                    continue;

                evaluated++;
                evaluateThinColumn(tau, colOf[px], lowRow, span, x, y, budget, bestSide, bestOp, bestTau);
            }
            else if (rowOf[px] == rowOf[py])
            {
                int span = abs(colOf[px] - colOf[py]);
                int lowCol = min(colOf[px], colOf[py]);

                if (2 * span > N)
                    continue;
                if (sumPref(prefV, rowOf[px], lowCol, 1, span) != 0)
                    continue;
                if (!snapshotCardValid(x, tau) || !snapshotCardValid(y, tau))
                    continue;

                evaluated++;
                evaluateThinRow(tau, rowOf[px], lowCol, span, x, y, budget, bestSide, bestOp, bestTau);
            }
            else
            {
                continue;
            }

            if (evaluated >= jumpEvalLimit)
                break;
            if (bestTau >= 0 && bestSide <= 0 && evaluated >= jumpEarlyEvalLimit)
                break;
        }

        if (bestTau < 0)
            return false;
        if (!applyRetroJump(bestOp, bestTau))
            return false;
        if (a[target] != target)
            return false;

        jumpRetroAcceptedRun++;
        return true;
    }

    bool tryChainFix(int target, int forwardCost)
    {
        if (!extensionsEnabled)
            return false;
        if (a[target] == target)
            return true;

        int x = target;
        int y = a[target];

        if (!alive[x] || !alive[y])
            return false;

        int px = pos[x];

        long long bestDelta = LLONG_MAX;
        int bestBridge = -1;
        Meet bestAnchor{};
        Meet bestLink{};

        for (int z = 0; z < M; z++)
        {
            anchorFoundScratch[z] = false;

            if (z == x || z == y || !alive[z])
                continue;

            Meet anchor;

            if (!bestMeetBelowInto(y, z, INT_MAX, anchor))
                continue;

            anchorFoundScratch[z] = true;
            anchorTimeScratch[z] = anchor.t;
            anchorMeetScratch[z] = anchor;

            Meet link;

            if (!bestMeetBelowInto(x, z, anchor.t, link))
                continue;

            int pz = pos[z];

            long long delta = 0;
            delta += distAll[pz][y] - distAll[target][y];
            delta += distAll[px][z] - distAll[pz][z];
            delta -= distAll[px][x];

            if (delta < bestDelta)
            {
                bestDelta = delta;
                bestBridge = z;
                bestAnchor = anchor;
                bestLink = link;
            }
        }

        if (bestBridge >= 0 && bestDelta + distAll[px][x] <= 3LL * forwardCost)
        {
            applyRetroInsertion(y, bestBridge, bestAnchor);

            if (!meetAccurate(bestLink, x, bestBridge))
                return false;

            applyRetroInsertion(x, bestBridge, bestLink);

            return a[target] == target;
        }

        if (!deepChainsEnabled || forwardCost < 4)
            return false;

        return tryDeepChain(x, y, px, forwardCost);
    }

    bool tryDeepChain(int x, int y, int px, int forwardCost)
    {
        array<pair<long long, int>, M> ranked;
        int rankedCount = 0;

        for (int z = 0; z < M; z++)
        {
            if (!anchorFoundScratch[z] || anchorTimeScratch[z] < 2)
                continue;

            int pz = pos[z];
            ranked[rankedCount++] = {(long long)distAll[pz][y] - distAll[pz][z], z};
        }

        if (rankedCount == 0)
            return false;

        sort(ranked.begin(), ranked.begin() + rankedCount);

        int firstLimit = min(rankedCount, deepBridgeLimit);

        memset(finalKnownScratch, 0, sizeof(finalKnownScratch));

        long long bestDelta = LLONG_MAX;
        int bestFirst = -1;
        int bestSecond = -1;
        Meet bestAnchor{};
        Meet bestMiddle{};
        Meet bestFinal{};

        for (int idx = 0; idx < firstLimit; idx++)
        {
            int firstBridge = ranked[idx].second;
            int firstTime = anchorTimeScratch[firstBridge];
            int firstPos = pos[firstBridge];

            long long partial = (long long)distAll[firstPos][y] - distAll[x][y] - distAll[firstPos][firstBridge] - distAll[px][x];

            for (int secondBridge = 0; secondBridge < M; secondBridge++)
            {
                if (secondBridge == x || secondBridge == y || secondBridge == firstBridge || !alive[secondBridge])
                    continue;

                if (!effectiveLastExists(x, secondBridge))
                    continue;

                if (!finalKnownScratch[secondBridge])
                {
                    finalKnownScratch[secondBridge] = 1;
                    collectFinalMeets(x, secondBridge);
                }

                if (finalCountScratch[secondBridge] == 0)
                    continue;
                if (finalEarliestScratch[secondBridge] + 2 > firstTime)
                    continue;

                if (!effectiveLastExists(firstBridge, secondBridge))
                    continue;

                Meet middle;

                if (!bestMeetBelowInto(firstBridge, secondBridge, firstTime, middle))
                    continue;
                if (finalEarliestScratch[secondBridge] >= middle.t)
                    continue;

                const Meet *finalMeet = nullptr;

                for (int k = finalCountScratch[secondBridge] - 1; k >= 0; k--)
                {
                    if (finalMeetScratch[secondBridge][k].t < middle.t)
                    {
                        finalMeet = &finalMeetScratch[secondBridge][k];
                        break;
                    }
                }

                if (!finalMeet)
                    continue;

                int secondPos = pos[secondBridge];

                long long delta = partial + distAll[secondPos][firstBridge] + distAll[px][secondBridge] - distAll[secondPos][secondBridge];

                if (delta < bestDelta)
                {
                    bestDelta = delta;
                    bestFirst = firstBridge;
                    bestSecond = secondBridge;
                    bestAnchor = anchorMeetScratch[firstBridge];
                    bestMiddle = middle;
                    bestFinal = *finalMeet;
                }
            }
        }

        if (bestFirst < 0)
            return false;

        if (bestDelta + distAll[px][x] > 3LL * forwardCost)
            return false;

        applyRetroInsertion(y, bestFirst, bestAnchor);

        if (!meetAccurate(bestMiddle, bestFirst, bestSecond))
            return false;

        applyRetroInsertion(bestFirst, bestSecond, bestMiddle);

        if (!meetAccurate(bestFinal, x, bestSecond))
            return false;

        applyRetroInsertion(x, bestSecond, bestFinal);

        deepChainAcceptedRun++;
        deepChainSavedRun += forwardCost - 3;

        return a[x] == x;
    }

    void applyOperationToState(const Operation &op)
    {
        if (op.d == 'V')
        {
            int half = op.h / 2;

            for (int x = 0; x < half; x++)
            {
                for (int y = 0; y < op.w; y++)
                {
                    int p = id(op.r + x, op.c + y);
                    int q = id(op.r + half + x, op.c + y);

                    swap(a[p], a[q]);

                    pos[a[p]] = p;
                    pos[a[q]] = q;
                }
            }
        }
        else
        {
            int half = op.w / 2;

            for (int x = 0; x < op.h; x++)
            {
                for (int y = 0; y < half; y++)
                {
                    int p = id(op.r + x, op.c + y);
                    int q = id(op.r + x, op.c + half + y);

                    swap(a[p], a[q]);

                    pos[a[p]] = p;
                    pos[a[q]] = q;
                }
            }
        }
    }

    void toggleCards(const Operation &op)
    {
        if (op.d == 'V')
        {
            int half = op.h / 2;

            for (int x = 0; x < half; x++)
            {
                int *upper = a + id(op.r + x, op.c);
                int *lower = a + id(op.r + half + x, op.c);
                swap_ranges(upper, upper + op.w, lower);
            }
        }
        else
        {
            int half = op.w / 2;

            for (int x = 0; x < op.h; x++)
            {
                int *left = a + id(op.r + x, op.c);
                swap_ranges(left, left + half, left + half);
            }
        }
    }

    void appendBaseOperation(const Operation &op)
    {
        if (extensionsEnabled)
        {
            snapshotHistory();
        }

        captureRectContacts(op);
        applyOperationToState(op);
        baseOps.push_back(op);
        writeBreaks((int)baseOps.size() - 1);
        refreshHeads((int)baseOps.size());
        shiftMidsAfterRect(op, (int)baseOps.size());
    }

    inline int activeCount(int r, int c, int h, int w) const
    {
        return prefA[r + h][c + w] - prefA[r][c + w] - prefA[r + h][c] + prefA[r][c];
    }

    int adjacentPathInto(int src, int tgt)
    {
        searchMarkCur++;
        int cur = searchMarkCur;

        searchMark[src] = cur;
        searchPrev[src] = src;
        searchQueue[0] = src;
        int qh = 0, qt = 1;

        while (qh < qt && searchMark[tgt] != cur)
        {
            int v = searchQueue[qh++];
            int cnt = neighborCount[v];

            for (int k = 0; k < cnt; k++)
            {
                int u = neighborCell[v][k];

                if (!alive[u] || searchMark[u] == cur)
                    continue;

                searchMark[u] = cur;
                searchPrev[u] = v;

                if (u == tgt)
                    break;

                searchQueue[qt++] = u;
            }
        }

        if (searchMark[tgt] != cur)
            return 0;

        int len = 0;
        int walk = tgt;

        while (walk != src)
        {
            pathCells[len++] = walk;
            walk = searchPrev[walk];
        }

        reverse(pathCells, pathCells + len);
        return len;
    }

    int macroPathInto(int src, int tgt)
    {
        if (src == tgt)
            return 0;

        searchMarkCur++;
        int cur = searchMarkCur;

        searchMark[src] = cur;
        searchPrev[src] = src;
        searchQueue[0] = src;
        int qh = 0, qt = 1;

        while (qh < qt && searchMark[tgt] != cur)
        {
            int v = searchQueue[qh++];
            int r = rowOf[v];
            int c = colOf[v];

            int s = colRunStart[c][r];
            int e = colRunEnd[c][r];
            int half = (e - s + 1) >> 1;
            int from = max(s, r - half);
            int to = min(e, r + half);

            for (int scanRow = from; scanRow <= to; scanRow++)
            {
                int u = id(scanRow, c);

                if (u == v || searchMark[u] == cur)
                    continue;

                searchMark[u] = cur;
                searchPrev[u] = v;

                if (u == tgt)
                    break;

                searchQueue[qt++] = u;
            }

            if (searchMark[tgt] == cur)
                break;

            s = rowRunStart[r][c];
            e = rowRunEnd[r][c];
            half = (e - s + 1) >> 1;
            from = max(s, c - half);
            to = min(e, c + half);

            for (int scanCol = from; scanCol <= to; scanCol++)
            {
                int u = id(r, scanCol);

                if (u == v || searchMark[u] == cur)
                    continue;

                searchMark[u] = cur;
                searchPrev[u] = v;

                if (u == tgt)
                    break;

                searchQueue[qt++] = u;
            }
        }

        if (searchMark[tgt] != cur)
            return adjacentPathInto(src, tgt);

        int len = 0;
        int walk = tgt;

        while (walk != src)
        {
            pathCells[len++] = walk;
            walk = searchPrev[walk];
        }

        reverse(pathCells, pathCells + len);
        return len;
    }

    void buildVerticalGain(int d, int rowLo, int rowHi)
    {
        for (int c = 0; c <= N; c++)
        {
            gainPref[rowLo][c] = 0;
        }

        for (int r = rowLo; r < rowHi; r++)
        {
            gainPref[r + 1][0] = 0;

            for (int c = 0; c < N; c++)
            {
                int p = id(r, c);
                int q = p + d * N;

                int upperCard = a[p];
                int lowerCard = a[q];

                int gain = distAll[p][lowerCard] + distAll[q][upperCard] -
                           distAll[p][upperCard] - distAll[q][lowerCard];

                gainPref[r + 1][c + 1] =
                    gainPref[r + 1][c] + gainPref[r][c + 1] - gainPref[r][c] + gain;
            }
        }
    }

    void buildHorizontalGain(int d, int colLo, int colHi)
    {
        for (int c = colLo; c <= colHi; c++)
        {
            gainPref[0][c] = 0;
        }

        for (int r = 0; r < N; r++)
        {
            gainPref[r + 1][colLo] = 0;

            for (int c = colLo; c < colHi; c++)
            {
                int p = id(r, c);
                int q = p + d;

                int leftCard = a[p];
                int rightCard = a[q];

                int gain = distAll[p][rightCard] + distAll[q][leftCard] -
                           distAll[p][leftCard] - distAll[q][rightCard];

                gainPref[r + 1][c + 1] =
                    gainPref[r + 1][c] + gainPref[r][c + 1] - gainPref[r][c] + gain;
            }
        }
    }

    inline int gainRectSum(int top, int left, int height, int width) const
    {
        return gainPref[top + height][left + width] - gainPref[top][left + width] -
               gainPref[top + height][left] + gainPref[top][left];
    }

    template <typename Consider>
    void enumerateMoveRects(int p, int q, Consider &&consider)
    {
        int pr = rowOf[p], pc = colOf[p];
        int qr = rowOf[q], qc = colOf[q];

        if (pc == qc && pr != qr)
        {
            int d = abs(pr - qr);
            int upper = min(pr, qr);
            int h = 2 * d;

            int lo = max(0, upper - d + 1);
            int hi = min(upper, N - h);

            if (lo <= hi)
            {
                buildVerticalGain(d, lo, hi + d);

                int reach[N];

                for (int rectTop = lo; rectTop <= hi; rectTop++)
                {
                    for (int c = N - 1; c >= 0; c--)
                    {
                        bool good = activeCount(rectTop, c, h, 1) == h &&
                                    sumPref(prefH, rectTop, c, h - 1, 1) == 0;

                        if (!good)
                        {
                            reach[c] = c - 1;
                        }
                        else if (c == N - 1 || sumPref(prefV, rectTop, c, h, 1) != 0 || reach[c + 1] < c + 1)
                        {
                            reach[c] = c;
                        }
                        else
                        {
                            reach[c] = reach[c + 1];
                        }
                    }

                    for (int rectLeft = 0; rectLeft <= pc; rectLeft++)
                    {
                        int minw = pc - rectLeft + 1;
                        int maxw = min(N - rectLeft, reach[rectLeft] - rectLeft + 1);

                        for (int w = minw; w <= maxw; w++)
                        {
                            consider(Operation{'V', rectTop, rectLeft, h, w}, gainRectSum(rectTop, rectLeft, d, w));
                        }
                    }
                }
            }
        }
        else if (pr == qr && pc != qc)
        {
            int d = abs(pc - qc);
            int left = min(pc, qc);
            int w = 2 * d;

            int lo = max(0, left - d + 1);
            int hi = min(left, N - w);

            if (lo <= hi)
            {
                buildHorizontalGain(d, lo, hi + d);

                int reach[N];

                for (int rectLeft = lo; rectLeft <= hi; rectLeft++)
                {
                    for (int r = N - 1; r >= 0; r--)
                    {
                        bool good = activeCount(r, rectLeft, 1, w) == w &&
                                    sumPref(prefV, r, rectLeft, 1, w - 1) == 0;

                        if (!good)
                        {
                            reach[r] = r - 1;
                        }
                        else if (r == N - 1 || sumPref(prefH, r, rectLeft, 1, w) != 0 || reach[r + 1] < r + 1)
                        {
                            reach[r] = r;
                        }
                        else
                        {
                            reach[r] = reach[r + 1];
                        }
                    }

                    for (int rectTop = 0; rectTop <= pr; rectTop++)
                    {
                        int minh = pr - rectTop + 1;
                        int maxh = min(N - rectTop, reach[rectTop] - rectTop + 1);

                        for (int h = minh; h <= maxh; h++)
                        {
                            consider(Operation{'H', rectTop, rectLeft, h, w}, gainRectSum(rectTop, rectLeft, h, d));
                        }
                    }
                }
            }
        }
    }

    Operation bestOpForMove(int p, int q)
    {
        Operation best{'H', 0, 0, 1, 2};

        bool found = false;
        long long bestScore = LLONG_MAX;
        int bestArea = -1;
        int tieCount = 1;

        enumerateMoveRects(p, q, [&](const Operation &op, int delta)
                           {
            int area = op.h * op.w;
            long long score = 1000LL * delta - 7LL * area;

            if (!found || score < bestScore || (score == bestScore && area > bestArea))
            {
                found = true;
                bestScore = score;
                bestArea = area;
                best = op;
                tieCount = 1;
            }
            else if (jitterEnabled && found && score == bestScore && area == bestArea)
            {
                tieCount++;

                if (randomBelow(tieCount) == 0)
                {
                    best = op;
                }
            } });

        if (found)
            return best;

        return edgeOperation(p, q);
    }

    long long bestScoreForMove(int p, int q)
    {
        bool found = false;
        long long bestScore = 0;

        enumerateMoveRects(p, q, [&](const Operation &op, int delta)
                           {
            long long score = 1000LL * delta - 7LL * (op.h * op.w);

            if (!found || score < bestScore)
            {
                found = true;
                bestScore = score;
            } });

        return bestScore;
    }

    int collectTopMoveRects(int p, int q, int limit, array<Candidate, MAXCAND> &out)
    {
        int cnt = 0;

        auto better = [](const Candidate &x, const Candidate &y)
        {
            if (x.score != y.score)
                return x.score < y.score;
            return x.area > y.area;
        };

        enumerateMoveRects(p, q, [&](const Operation &op, int delta)
                           {
            Candidate cand{1000LL * delta - 7LL * (op.h * op.w), op.h * op.w, op};

            if (cnt < limit)
            {
                out[cnt++] = cand;

                for (int i = cnt - 1; i > 0 && better(out[i], out[i - 1]); i--)
                {
                    swap(out[i], out[i - 1]);
                }
            }
            else if (better(cand, out[limit - 1]))
            {
                out[limit - 1] = cand;

                for (int i = limit - 1; i > 0 && better(out[i], out[i - 1]); i--)
                {
                    swap(out[i], out[i - 1]);
                }
            } });

        return cnt;
    }

    Operation bestOpForJumpPair(int p, int firstStop, int secondStop)
    {
        array<Candidate, MAXCAND> cands;
        int limit = max(2, min(firstCand, MAXCAND));
        int cnt = collectTopMoveRects(p, firstStop, limit, cands);

        if (cnt == 0)
            return edgeOperation(p, firstStop);
        if (cnt == 1)
            return cands[0].op;

        int bestIdx = 0;
        long long bestJoint = LLONG_MAX;

        for (int i = 0; i < cnt; i++)
        {
            if (elapsedMs() > abortLimitMs)
            {
                aborted = true;
                break;
            }

            toggleCards(cands[i].op);
            long long joint = cands[i].score + bestScoreForMove(firstStop, secondStop);
            toggleCards(cands[i].op);

            if (joint < bestJoint)
            {
                bestJoint = joint;
                bestIdx = i;
            }
        }

        return cands[bestIdx].op;
    }

    int ensureTree(int root)
    {
        auto found = treeCacheIndex.find(root);

        if (found != treeCacheIndex.end())
            return found->second;

        vector<int> par(M, -1);
        vector<int> depth(M, -1);

        int qh = 0, qt = 0;
        depth[root] = 0;
        searchQueue[qt++] = root;

        while (qh < qt)
        {
            int v = searchQueue[qh++];

            for (int u : adj[v])
            {
                if (depth[u] == -1)
                {
                    depth[u] = depth[v] + 1;
                    par[u] = v;
                    searchQueue[qt++] = u;
                }
            }
        }

        vector<int> childCnt(M, 0);

        for (int v = 0; v < M; v++)
        {
            if (par[v] != -1)
            {
                childCnt[par[v]]++;
            }
        }

        vector<int> leaves;

        for (int v = 0; v < M; v++)
        {
            if (v != root && childCnt[v] == 0)
            {
                leaves.push_back(v);
            }
        }

        int slot = (int)treeParentCache.size();
        treeParentCache.push_back(move(par));
        treeDepthCache.push_back(move(depth));
        treeChildCache.push_back(move(childCnt));
        treeLeafListCache.push_back(move(leaves));
        treeCacheIndex[root] = slot;

        return slot;
    }

    void addLeaf(int v)
    {
        leafCells[leafCount] = v;
        leafSlot[v] = leafCount;
        leafCount++;
    }

    void removeLeaf(int v)
    {
        int i = leafSlot[v];
        int last = leafCount - 1;
        int moved = leafCells[last];

        leafCells[i] = moved;
        leafSlot[moved] = i;
        leafCount--;
    }

    int chooseLeaf(const int *depth, int mode)
    {
        array<array<long long, 5>, jitterChoices> topKeys;
        array<int, jitterChoices> topTargets;
        int topCount = 0;

        for (int idx = 0; idx < leafCount; idx++)
        {
            int v = leafCells[idx];

            int cheap;

            if (a[v] == v)
            {
                cheap = 3;
            }
            else
            {
                cheap = validMeet(v, a[v]) ? 2 : 0;
            }

            int dcur = distAll[pos[v]][v];
            int misAtTarget = (a[v] == v ? 0 : 1);

            array<long long, 5> key;

            if (mode == 0)
            {
                key = {cheap, depth[v], -dcur, -misAtTarget, -v};
            }
            else if (mode == 1)
            {
                key = {depth[v], cheap, -dcur, -misAtTarget, -v};
            }
            else if (mode == 2)
            {
                key = {cheap, -dcur, depth[v], -misAtTarget, -v};
            }
            else
            {
                key = {depth[v], -dcur, cheap, -misAtTarget, -v};
            }

            int slot = topCount;

            while (slot > 0 && key > topKeys[slot - 1])
            {
                slot--;
            }

            if (slot >= jitterChoices)
                continue;

            int last = min(topCount, jitterChoices - 1);

            for (int k = last; k > slot; k--)
            {
                topKeys[k] = topKeys[k - 1];
                topTargets[k] = topTargets[k - 1];
            }

            topKeys[slot] = key;
            topTargets[slot] = v;

            if (topCount < jitterChoices)
            {
                topCount++;
            }
        }

        if (topCount == 0)
            return -1;

        int pick = 0;

        if (topCount > 1 && jitterRoll())
        {
            pick = randomBelow(topCount);
        }

        return topTargets[pick];
    }

    void forceAdjacentMove(int target)
    {
        while (a[target] != target)
        {
            if (extensionsEnabled && tryPastFix(target))
                break;

            int len = adjacentPathInto(pos[target], target);

            if (len == 0)
                break;

            if (extensionsEnabled && len >= 2 && tryPastMacroFix(target, len))
                break;

            if (extensionsEnabled && len >= 3 && tryChainFix(target, len))
                break;

            for (int idx = 0; idx < len; idx++)
            {
                if (a[target] == target)
                    break;

                Operation op = edgeOperation(pos[target], pathCells[idx]);
                appendBaseOperation(op);

                if (tryPastFix(target))
                    break;
            }
        }
    }

    void moveTarget(int target)
    {
        int guard = 0;

        while (a[target] != target && guard++ < 1000)
        {
            if (aborted)
                return;

            if (tryPastFix(target))
                return;

            int steps = macroPathInto(pos[target], target);

            if (extensionsEnabled)
            {
                int forwardCost = steps == 0 ? 8 : steps;

                if (forwardCost >= 2 && tryPastMacroFix(target, forwardCost))
                    return;

                if (forwardCost >= 3 && tryChainFix(target, forwardCost))
                    return;
            }

            if (steps == 0)
                break;

            for (int idx = 0; idx < steps; idx++)
            {
                int nxt = pathCells[idx];

                if (a[target] == target)
                    return;
                if (tryPastFix(target))
                    return;

                if (extensionsEnabled && idx > 0)
                {
                    int remaining = steps - idx;

                    if (remaining >= 2 && tryPastMacroFix(target, remaining))
                        return;

                    if (remaining >= 3 && tryChainFix(target, remaining))
                        return;
                }

                Operation op;

                if (lookaheadEnabled && idx + 1 < steps)
                {
                    op = bestOpForJumpPair(pos[target], nxt, pathCells[idx + 1]);
                }
                else
                {
                    op = bestOpForMove(pos[target], nxt);
                }

                if (aborted)
                    return;

                appendBaseOperation(op);

                if (tryPastFix(target))
                    return;
            }
        }

        if (aborted)
            return;

        if (a[target] != target)
        {
            forceAdjacentMove(target);
        }
    }

    vector<Operation> flattenAndCompress()
    {
        sort(retroOps.begin(), retroOps.end(), [](const TimedOperation &x, const TimedOperation &y)
             {
            if (x.t != y.t) return x.t < y.t;
            return x.ord < y.ord; });

        vector<Operation> flat;
        flat.reserve(baseOps.size() + retroOps.size());

        size_t ptr = 0;

        auto emitRetro = [&](int t)
        {
            while (ptr < retroOps.size() && retroOps[ptr].t == t)
            {
                flat.push_back(retroOps[ptr].op);
                ptr++;
            }
        };

        emitRetro(0);

        for (int i = 0; i < (int)baseOps.size(); i++)
        {
            flat.push_back(baseOps[i]);
            emitRetro(i + 1);
        }

        while (ptr < retroOps.size())
        {
            flat.push_back(retroOps[ptr].op);
            ptr++;
        }

        vector<Operation> comp;
        comp.reserve(flat.size());

        for (const auto &op : flat)
        {
            if (!comp.empty() && sameOp(comp.back(), op))
            {
                comp.pop_back();
            }
            else
            {
                comp.push_back(op);
            }
        }

        return comp;
    }

    bool verifyOps(const vector<Operation> &ops) const
    {
        if ((int)ops.size() > 100000)
            return false;

        vector<int> board = initA;

        for (const auto &op : ops)
        {
            if (op.d != 'V' && op.d != 'H')
                return false;
            if (op.r < 0 || op.c < 0 || op.h <= 0 || op.w <= 0)
                return false;
            if (op.r + op.h > N || op.c + op.w > N)
                return false;
            if (op.d == 'V' && (op.h % 2) != 0)
                return false;
            if (op.d == 'H' && (op.w % 2) != 0)
                return false;

            if (op.w >= 2 && sumPref(prefV, op.r, op.c, op.h, op.w - 1) != 0)
                return false;
            if (op.h >= 2 && sumPref(prefH, op.r, op.c, op.h - 1, op.w) != 0)
                return false;

            if (op.d == 'V')
            {
                int half = op.h / 2;

                for (int x = 0; x < half; x++)
                {
                    for (int y = 0; y < op.w; y++)
                    {
                        swap(board[id(op.r + x, op.c + y)], board[id(op.r + half + x, op.c + y)]);
                    }
                }
            }
            else
            {
                int half = op.w / 2;

                for (int x = 0; x < op.h; x++)
                {
                    for (int y = 0; y < half; y++)
                    {
                        swap(board[id(op.r + x, op.c + y)], board[id(op.r + x, op.c + half + y)]);
                    }
                }
            }
        }

        for (int i = 0; i < M; i++)
        {
            if (board[i] != i)
                return false;
        }

        return true;
    }

    vector<Operation> solveOnce(int root, int mode, bool useExtensions)
    {
        initRun(useExtensions);

        int slot = ensureTree(root);
        const int *par = treeParentCache[slot].data();
        const int *depth = treeDepthCache[slot].data();

        memcpy(childCount, treeChildCache[slot].data(), sizeof(childCount));

        const vector<int> &leaves = treeLeafListCache[slot];

        leafCount = 0;

        for (int v : leaves)
        {
            addLeaf(v);
        }

        for (int fixed = 0; fixed < M - 1; fixed++)
        {
            if (elapsedMs() > abortLimitMs)
            {
                aborted = true;
                return {};
            }

            if (leafCount == 0)
                break;

            int target = chooseLeaf(depth, mode);

            if (a[target] != target)
            {
                moveTarget(target);
            }

            if (aborted)
                return {};

            if (a[target] != target)
            {
                forceAdjacentMove(target);
            }

            alive[target] = false;
            removeLeaf(target);
            applyCellDeath(target);

            int p = par[target];

            if (p != -1)
            {
                childCount[p]--;

                if (p != root && childCount[p] == 0)
                {
                    addLeaf(p);
                }
            }
        }

        return flattenAndCompress();
    }

    vector<pair<pair<int, int>, int>> rootRanking() const
    {
        vector<pair<pair<int, int>, int>> rs;
        rs.reserve(M);

        for (int s = 0; s < M; s++)
        {
            int ecc = 0;
            int sum = 0;

            for (int t = 0; t < M; t++)
            {
                ecc = max(ecc, (int)distAll[s][t]);
                sum += distAll[s][t];
            }

            rs.push_back({{ecc, sum}, s});
        }

        sort(rs.begin(), rs.end());
        return rs;
    }
};

struct SequenceCompressor
{
    static constexpr int N = 20;
    static constexpr int boxCellLimit = 16;
    static constexpr int windowLengthLimit = 8;
    static constexpr int windowMemberLimit = 8;
    static constexpr int pairTableLimit = 128;
    static constexpr int deepPairLimit = 88;
    static constexpr int gatherScanLimit = 30;

    struct BoxCatalog
    {
        int top, left, height, width, area;
        vector<Operation> actions;
        vector<array<uint8_t, boxCellLimit>> arrangements;
        vector<pair<uint64_t, int>> lookup;
        vector<pair<uint64_t, pair<int, int>>> forwardPairs;
        bool forwardBuilt = false;
        uint64_t identityCode;
    };

    const Solver &solver;
    chrono::steady_clock::time_point origin;
    double deadlineMs;
    unordered_map<int, BoxCatalog> catalogs;

    long long cancelSaved = 0;
    long long mergeSaved = 0;
    long long windowSaved = 0;
    long long gatherSaved = 0;

    SequenceCompressor(const Solver &solverRef, chrono::steady_clock::time_point startPoint, double deadline)
        : solver(solverRef), origin(startPoint), deadlineMs(deadline) {}

    double elapsedMs() const
    {
        return chrono::duration<double, milli>(chrono::steady_clock::now() - origin).count();
    }

    bool timeUp() const
    {
        return elapsedMs() > deadlineMs;
    }

    static bool rectanglesOverlap(const Operation &a, const Operation &b)
    {
        return a.r < b.r + b.h && b.r < a.r + a.h && a.c < b.c + b.w && b.c < a.c + a.w;
    }

    static bitset<400> footprintOf(const Operation &op)
    {
        bitset<400> bits;
        for (int row = op.r; row < op.r + op.h; ++row)
            for (int col = op.c; col < op.c + op.w; ++col)
                bits.set(row * N + col);
        return bits;
    }

    static bitset<400> bandOf(const Operation &op)
    {
        bitset<400> bits;
        if (op.d == 'V')
        {
            for (int row = op.r; row < op.r + op.h; ++row)
                for (int col = 0; col < N; ++col)
                    bits.set(row * N + col);
        }
        else
        {
            for (int col = op.c; col < op.c + op.w; ++col)
                for (int row = 0; row < N; ++row)
                    bits.set(row * N + col);
        }
        return bits;
    }

    static void applyToArrangement(array<uint8_t, boxCellLimit> &arrangement, const Operation &op, int top, int left, int width)
    {
        if (op.d == 'V')
        {
            int half = op.h / 2;
            for (int x = 0; x < half; ++x)
            {
                for (int y = 0; y < op.w; ++y)
                {
                    int upperCell = (op.r - top + x) * width + (op.c - left + y);
                    int lowerCell = upperCell + half * width;
                    swap(arrangement[upperCell], arrangement[lowerCell]);
                }
            }
        }
        else
        {
            int half = op.w / 2;
            for (int x = 0; x < op.h; ++x)
            {
                for (int y = 0; y < half; ++y)
                {
                    int leftCell = (op.r - top + x) * width + (op.c - left + y);
                    int rightCell = leftCell + half;
                    swap(arrangement[leftCell], arrangement[rightCell]);
                }
            }
        }
    }

    static uint64_t encodeArrangement(const array<uint8_t, boxCellLimit> &arrangement, int area)
    {
        uint64_t code = 0;
        for (int cell = 0; cell < area; ++cell)
            code |= (uint64_t)arrangement[cell] << (4 * cell);
        return code;
    }

    static int findCode(const vector<pair<uint64_t, int>> &table, uint64_t code)
    {
        auto it = lower_bound(table.begin(), table.end(), make_pair(code, INT_MIN));
        if (it != table.end() && it->first == code)
            return it->second;
        return -1;
    }

    static int findPairCode(const vector<pair<uint64_t, pair<int, int>>> &table, uint64_t code)
    {
        auto it = lower_bound(table.begin(), table.end(), make_pair(code, make_pair(INT_MIN, INT_MIN)));
        if (it != table.end() && it->first == code)
            return (int)(it - table.begin());
        return -1;
    }

    BoxCatalog &catalogFor(int top, int left, int height, int width)
    {
        int key = ((top * N + left) * (N + 1) + height) * (N + 1) + width;
        auto found = catalogs.find(key);
        if (found != catalogs.end())
            return found->second;

        BoxCatalog catalog;
        catalog.top = top;
        catalog.left = left;
        catalog.height = height;
        catalog.width = width;
        catalog.area = height * width;

        for (int r = top; r < top + height; ++r)
        {
            for (int c = left; c < left + width; ++c)
            {
                for (int h = 1; r + h <= top + height; ++h)
                {
                    for (int w = 1; c + w <= left + width; ++w)
                    {
                        if (!solver.wallFree(r, c, h, w))
                            continue;
                        if (h % 2 == 0)
                            catalog.actions.push_back({'V', r, c, h, w});
                        if (w % 2 == 0)
                            catalog.actions.push_back({'H', r, c, h, w});
                    }
                }
            }
        }

        int count = (int)catalog.actions.size();
        catalog.arrangements.reserve(count);
        catalog.lookup.reserve(count);

        for (int k = 0; k < count; ++k)
        {
            array<uint8_t, boxCellLimit> arrangement;
            for (int cell = 0; cell < boxCellLimit; ++cell)
                arrangement[cell] = (uint8_t)cell;
            applyToArrangement(arrangement, catalog.actions[k], top, left, width);
            catalog.arrangements.push_back(arrangement);
            catalog.lookup.push_back({encodeArrangement(arrangement, catalog.area), k});
        }

        sort(catalog.lookup.begin(), catalog.lookup.end());

        array<uint8_t, boxCellLimit> identityArrangement;
        for (int cell = 0; cell < boxCellLimit; ++cell)
            identityArrangement[cell] = (uint8_t)cell;
        catalog.identityCode = encodeArrangement(identityArrangement, catalog.area);

        return catalogs.emplace(key, move(catalog)).first->second;
    }

    void ensureForwardPairs(BoxCatalog &catalog)
    {
        if (catalog.forwardBuilt)
            return;

        catalog.forwardBuilt = true;

        int count = (int)catalog.actions.size();
        catalog.forwardPairs.reserve((size_t)count * (count - 1));

        for (int first = 0; first < count; ++first)
        {
            for (int second = 0; second < count; ++second)
            {
                if (second == first)
                    continue;

                array<uint8_t, boxCellLimit> arrangement = catalog.arrangements[first];
                applyToArrangement(arrangement, catalog.actions[second], catalog.top, catalog.left, catalog.width);
                catalog.forwardPairs.push_back({encodeArrangement(arrangement, catalog.area), {first, second}});
            }
        }

        sort(catalog.forwardPairs.begin(), catalog.forwardPairs.end());
    }

    bool solveWindow(BoxCatalog &catalog, const array<uint8_t, boxCellLimit> &target, int limit, vector<Operation> &replacement)
    {
        uint64_t targetCode = encodeArrangement(target, catalog.area);

        if (targetCode == catalog.identityCode)
        {
            replacement.clear();
            return true;
        }

        int direct = findCode(catalog.lookup, targetCode);
        if (direct >= 0)
        {
            replacement = {catalog.actions[direct]};
            return true;
        }

        if (limit < 2)
            return false;

        int count = (int)catalog.actions.size();
        vector<pair<uint64_t, int>> closingCodes(count);

        for (int k = 0; k < count; ++k)
        {
            array<uint8_t, boxCellLimit> arrangement = target;
            applyToArrangement(arrangement, catalog.actions[k], catalog.top, catalog.left, catalog.width);
            uint64_t code = encodeArrangement(arrangement, catalog.area);
            closingCodes[k] = {code, k};

            int opener = findCode(catalog.lookup, code);
            if (opener >= 0)
            {
                replacement = {catalog.actions[opener], catalog.actions[k]};
                return true;
            }
        }

        if (limit < 3 || count > pairTableLimit)
            return false;

        ensureForwardPairs(catalog);

        for (int k = 0; k < count; ++k)
        {
            int hit = findPairCode(catalog.forwardPairs, closingCodes[k].first);
            if (hit >= 0)
            {
                auto [first, second] = catalog.forwardPairs[hit].second;
                replacement = {catalog.actions[first], catalog.actions[second], catalog.actions[closingCodes[k].second]};
                return true;
            }
        }

        if (limit < 4 || count > deepPairLimit)
            return false;

        for (int outer = 0; outer < count; ++outer)
        {
            if ((outer & 7) == 0 && timeUp())
                return false;

            array<uint8_t, boxCellLimit> midway = target;
            applyToArrangement(midway, catalog.actions[outer], catalog.top, catalog.left, catalog.width);

            for (int inner = 0; inner < count; ++inner)
            {
                if (inner == outer)
                    continue;

                array<uint8_t, boxCellLimit> arrangement = midway;
                applyToArrangement(arrangement, catalog.actions[inner], catalog.top, catalog.left, catalog.width);
                uint64_t code = encodeArrangement(arrangement, catalog.area);

                int hit = findPairCode(catalog.forwardPairs, code);
                if (hit >= 0)
                {
                    auto [first, second] = catalog.forwardPairs[hit].second;
                    replacement = {catalog.actions[first], catalog.actions[second], catalog.actions[inner], catalog.actions[outer]};
                    return true;
                }
            }
        }

        return false;
    }

    bool cancellationSweep(vector<Operation> &sequence)
    {
        int count = (int)sequence.size();
        vector<char> dropped(count, 0);
        bool changed = false;

        for (int i = 0; i < count; ++i)
        {
            if ((i & 63) == 0 && timeUp())
                break;
            if (dropped[i])
                continue;

            for (int j = i + 1; j < count; ++j)
            {
                if (dropped[j])
                    continue;
                if (!rectanglesOverlap(sequence[i], sequence[j]))
                    continue;

                if (sameOp(sequence[i], sequence[j]))
                {
                    dropped[i] = dropped[j] = 1;
                    changed = true;
                    cancelSaved += 2;
                }
                break;
            }
        }

        if (changed)
        {
            vector<Operation> kept;
            kept.reserve(count);
            for (int i = 0; i < count; ++i)
                if (!dropped[i])
                    kept.push_back(sequence[i]);
            sequence.swap(kept);
        }

        return changed;
    }

    bool mergeSweep(vector<Operation> &sequence)
    {
        int count = (int)sequence.size();
        vector<bitset<400>> footprints(count);
        for (int i = 0; i < count; ++i)
            footprints[i] = footprintOf(sequence[i]);

        vector<char> consumed(count, 0);
        vector<Operation> rewritten;
        rewritten.reserve(count);
        bool changed = false;

        int index = 0;
        for (; index < count; ++index)
        {
            if ((index & 15) == 0 && timeUp())
                break;
            if (consumed[index])
                continue;

            Operation leader = sequence[index];
            vector<Operation> members;
            members.push_back(leader);

            bitset<400> blocked;
            bitset<400> band = bandOf(leader);

            for (int j = index + 1; j < count; ++j)
            {
                if (consumed[j])
                    continue;

                const Operation &candidate = sequence[j];
                bool joins = false;

                if (candidate.d == leader.d)
                {
                    bool sharesBand = (leader.d == 'V')
                                          ? (candidate.r == leader.r && candidate.h == leader.h)
                                          : (candidate.c == leader.c && candidate.w == leader.w);

                    if (sharesBand)
                    {
                        bool clashes = false;
                        for (const Operation &member : members)
                        {
                            if (rectanglesOverlap(member, candidate))
                            {
                                clashes = true;
                                break;
                            }
                        }
                        if (!clashes && (footprints[j] & blocked).none())
                            joins = true;
                    }
                }

                if (joins)
                {
                    members.push_back(candidate);
                    consumed[j] = 1;
                }
                else
                {
                    blocked |= footprints[j];
                    if ((blocked & band) == band)
                        break;
                }
            }

            if ((int)members.size() == 1)
            {
                rewritten.push_back(leader);
                continue;
            }

            if (leader.d == 'V')
            {
                sort(members.begin(), members.end(), [](const Operation &x, const Operation &y)
                     { return x.c < y.c; });

                int runColumn = members[0].c;
                int runWidth = members[0].w;
                int runSize = 1;

                for (size_t k = 1; k <= members.size(); ++k)
                {
                    if (k < members.size() && members[k].c == runColumn + runWidth &&
                        solver.wallFree(leader.r, runColumn, leader.h, runWidth + members[k].w))
                    {
                        runWidth += members[k].w;
                        ++runSize;
                    }
                    else
                    {
                        rewritten.push_back({'V', leader.r, runColumn, leader.h, runWidth});
                        if (runSize > 1)
                        {
                            changed = true;
                            mergeSaved += runSize - 1;
                        }
                        if (k < members.size())
                        {
                            runColumn = members[k].c;
                            runWidth = members[k].w;
                            runSize = 1;
                        }
                    }
                }
            }
            else
            {
                sort(members.begin(), members.end(), [](const Operation &x, const Operation &y)
                     { return x.r < y.r; });

                int runRow = members[0].r;
                int runHeight = members[0].h;
                int runSize = 1;

                for (size_t k = 1; k <= members.size(); ++k)
                {
                    if (k < members.size() && members[k].r == runRow + runHeight &&
                        solver.wallFree(runRow, leader.c, runHeight + members[k].h, leader.w))
                    {
                        runHeight += members[k].h;
                        ++runSize;
                    }
                    else
                    {
                        rewritten.push_back({'H', runRow, leader.c, runHeight, leader.w});
                        if (runSize > 1)
                        {
                            changed = true;
                            mergeSaved += runSize - 1;
                        }
                        if (k < members.size())
                        {
                            runRow = members[k].r;
                            runHeight = members[k].h;
                            runSize = 1;
                        }
                    }
                }
            }
        }

        for (; index < count; ++index)
            if (!consumed[index])
                rewritten.push_back(sequence[index]);

        if (changed)
            sequence.swap(rewritten);

        return changed;
    }

    bool windowSweep(vector<Operation> &sequence)
    {
        bool changed = false;
        int index = 0;

        while (index < (int)sequence.size())
        {
            if (timeUp())
                break;

            bool replaced = false;
            int top = sequence[index].r;
            int bottom = sequence[index].r + sequence[index].h;
            int left = sequence[index].c;
            int right = sequence[index].c + sequence[index].w;
            int maxLength = min(windowLengthLimit, (int)sequence.size() - index);

            for (int length = 2; length <= maxLength; ++length)
            {
                const Operation &tail = sequence[index + length - 1];
                top = min(top, tail.r);
                bottom = max(bottom, tail.r + tail.h);
                left = min(left, tail.c);
                right = max(right, tail.c + tail.w);

                int height = bottom - top;
                int width = right - left;

                if (height * width > boxCellLimit)
                    break;

                BoxCatalog &catalog = catalogFor(top, left, height, width);

                array<uint8_t, boxCellLimit> target;
                for (int cell = 0; cell < boxCellLimit; ++cell)
                    target[cell] = (uint8_t)cell;
                for (int k = 0; k < length; ++k)
                    applyToArrangement(target, sequence[index + k], top, left, width);

                int limit = min(length - 1, 4);
                if ((int)catalog.actions.size() > pairTableLimit)
                    limit = min(limit, 2);

                vector<Operation> replacement;
                if (solveWindow(catalog, target, limit, replacement))
                {
                    windowSaved += length - (long long)replacement.size();

                    sequence.erase(sequence.begin() + index, sequence.begin() + index + length);
                    sequence.insert(sequence.begin() + index, replacement.begin(), replacement.end());
                    changed = true;
                    replaced = true;
                    break;
                }
            }

            if (!replaced)
                ++index;
        }

        return changed;
    }

    bool tryGatherAt(vector<Operation> &sequence, int index)
    {
        int count = (int)sequence.size();
        const Operation &head = sequence[index];

        int top = head.r;
        int bottom = head.r + head.h;
        int left = head.c;
        int right = head.c + head.w;

        if ((bottom - top) * (right - left) > boxCellLimit)
            return false;

        bitset<400> memberUnion = footprintOf(head);
        vector<int> memberIdx;
        vector<Operation> members;
        memberIdx.push_back(index);
        members.push_back(head);

        bool contiguous = true;
        int scanEnd = min(count, index + 1 + gatherScanLimit);

        for (int j = index + 1; j < scanEnd; ++j)
        {
            if ((j & 7) == 0 && timeUp())
                return false;

            bitset<400> candidateFootprint = footprintOf(sequence[j]);

            if ((candidateFootprint & memberUnion).none())
            {
                contiguous = false;
                continue;
            }

            const Operation &candidate = sequence[j];
            int newTop = min(top, candidate.r);
            int newBottom = max(bottom, candidate.r + candidate.h);
            int newLeft = min(left, candidate.c);
            int newRight = max(right, candidate.c + candidate.w);

            if ((newBottom - newTop) * (newRight - newLeft) > boxCellLimit)
                break;
            if ((int)members.size() >= windowMemberLimit)
                break;

            top = newTop;
            bottom = newBottom;
            left = newLeft;
            right = newRight;
            memberUnion |= candidateFootprint;
            memberIdx.push_back(j);
            members.push_back(candidate);

            if (contiguous)
                continue;

            int height = bottom - top;
            int width = right - left;

            BoxCatalog &catalog = catalogFor(top, left, height, width);

            array<uint8_t, boxCellLimit> target;
            for (int cell = 0; cell < boxCellLimit; ++cell)
                target[cell] = (uint8_t)cell;
            for (const Operation &member : members)
                applyToArrangement(target, member, top, left, width);

            int limit = min((int)members.size() - 1, 4);
            if ((int)catalog.actions.size() > pairTableLimit)
                limit = min(limit, 2);

            vector<Operation> replacement;
            if (solveWindow(catalog, target, limit, replacement))
            {
                gatherSaved += (long long)members.size() - (long long)replacement.size();

                vector<Operation> rebuilt;
                rebuilt.reserve(sequence.size() - members.size() + replacement.size());

                int memberPtr = 0;
                int lastMember = memberIdx.back();

                for (int k = 0; k < count; ++k)
                {
                    bool isMember = memberPtr < (int)memberIdx.size() && memberIdx[memberPtr] == k;

                    if (isMember)
                        memberPtr++;

                    if (k == lastMember)
                    {
                        for (const Operation &op : replacement)
                            rebuilt.push_back(op);
                    }

                    if (!isMember)
                        rebuilt.push_back(sequence[k]);
                }

                sequence.swap(rebuilt);
                return true;
            }
        }

        return false;
    }

    bool gatherSweep(vector<Operation> &sequence)
    {
        bool changed = false;
        int index = 0;

        while (index < (int)sequence.size())
        {
            if (timeUp())
                break;

            if (tryGatherAt(sequence, index))
                changed = true;
            else
                ++index;
        }

        return changed;
    }

    void run(vector<Operation> &sequence)
    {
        while (!timeUp())
        {
            bool changed = false;
            while (!timeUp() && cancellationSweep(sequence))
                changed = true;
            if (!timeUp() && mergeSweep(sequence))
                changed = true;
            if (!changed)
                break;
        }

        while (!timeUp())
        {
            bool changed = windowSweep(sequence);
            if (!timeUp() && gatherSweep(sequence))
                changed = true;
            while (!timeUp() && cancellationSweep(sequence))
                changed = true;
            if (!timeUp() && mergeSweep(sequence))
                changed = true;
            if (!changed)
                break;
        }
    }
};

int main()
{
    auto programStart = chrono::steady_clock::now();

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int gridSize;
    cin >> gridSize;

    vector<int> initialCards(400);

    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            cin >> initialCards[i * 20 + j];
        }
    }

    vector<string> verticalWalls(20), horizontalWalls(19);

    for (int i = 0; i < 20; i++)
    {
        cin >> verticalWalls[i];
    }

    for (int i = 0; i < 19; i++)
    {
        cin >> horizontalWalls[i];
    }

    Solver solver(initialCards, verticalWalls, horizontalWalls);
    solver.clockStart = programStart;

    auto ranking = solver.rootRanking();

    vector<int> roots;

    for (int i = 0; i < (int)ranking.size() && i < 10; i++)
    {
        roots.push_back(ranking[i].second);
    }

    int extraRoots[] = {
        0, 19, 380, 399,
        9, 10, 190, 209, 210};

    for (int x : extraRoots)
    {
        roots.push_back(x);
    }

    sort(roots.begin(), roots.end());
    roots.erase(unique(roots.begin(), roots.end()), roots.end());

    vector<pair<int, int>> schedule;

    for (int mode = 0; mode < 4; mode++)
    {
        for (int root : roots)
        {
            schedule.push_back({root, mode});
        }
    }

    {
        set<int> primary(roots.begin(), roots.end());
        int mode = 0;

        for (int i = 0; i < (int)ranking.size(); i++)
        {
            int root = ranking[i].second;

            if (primary.count(root))
                continue;

            schedule.push_back({root, mode});
            mode = (mode + 1) % 4;
        }
    }

    vector<Operation> best;
    vector<Operation> firstRun;
    bool have = false;
    int bestRoot = ranking[0].second;
    int bestMode = 0;
    int runIndex = 0;
    int completedRuns = 0;
    double baseRunMs = 1.0;
    long long deepAcceptedTotal = 0;
    long long deepSavedTotal = 0;
    long long jumpRetroTotal = 0;
    long long bestDeepAccepted = 0;
    long long bestDeepSaved = 0;

    auto attempt = [&](int root, int mode, bool useExtensions, bool useJitter, bool useLookahead)
    {
        solver.jitterEnabled = useJitter;
        solver.lookaheadEnabled = useLookahead;
        solver.abortLimitMs = have ? 1845.0 : 1e18;
        solver.seedRandom(splitMix((uint64_t)runIndex + 1));
        runIndex++;

        double runStart = solver.elapsedMs();
        vector<Operation> cand = solver.solveOnce(root, mode, useExtensions);
        double runMs = solver.elapsedMs() - runStart;

        deepAcceptedTotal += solver.deepChainAcceptedRun;
        deepSavedTotal += solver.deepChainSavedRun;
        jumpRetroTotal += solver.jumpRetroAcceptedRun;

        if (useLookahead && runMs > 2.0 * baseRunMs && solver.firstCand > 2)
        {
            solver.firstCand--;
        }

        if (solver.aborted)
            return;
        if (!solver.verifyOps(cand))
            return;

        completedRuns++;

        if (!have || cand.size() < best.size())
        {
            best = move(cand);
            have = true;
            bestRoot = root;
            bestMode = mode;
            bestDeepAccepted = solver.deepChainAcceptedRun;
            bestDeepSaved = solver.deepChainSavedRun;
        }
    };

    {
        double runStart = solver.elapsedMs();
        attempt(ranking[0].second, 0, false, false, false);
        baseRunMs = max(1.0, solver.elapsedMs() - runStart);
        firstRun = best;
    }

    for (auto [root, mode] : schedule)
    {
        if (have && solver.elapsedMs() > 1830.0)
            break;

        attempt(root, mode, true, true, true);
    }

    while (have && solver.elapsedMs() < 1830.0)
    {
        attempt(bestRoot, bestMode, true, true, true);
    }

    if (!have)
    {
        solver.jitterEnabled = false;
        solver.lookaheadEnabled = false;
        solver.abortLimitMs = 1e18;
        best = solver.solveOnce(ranking[0].second, 1, false);
        have = true;

        if (firstRun.empty())
        {
            firstRun = best;
        }
    }

    if ((int)best.size() > 100000)
    {
        solver.jitterEnabled = false;
        solver.lookaheadEnabled = false;
        solver.abortLimitMs = 1e18;

        vector<Operation> fallback = solver.solveOnce(ranking[0].second, 1, false);

        if (!solver.aborted && solver.verifyOps(fallback) && fallback.size() < best.size())
        {
            best = move(fallback);
        }
    }

    size_t rawCount = best.size();

    SequenceCompressor compressor(solver, programStart, 1940.0);
    vector<Operation> compressed = best;
    compressor.run(compressed);

    vector<Operation> output = best;

    if (compressed.size() < best.size() && solver.verifyOps(compressed))
    {
        output = move(compressed);
    }

    if (!solver.verifyOps(output))
    {
        output = best;
    }

    if (!solver.verifyOps(output))
    {
        output = firstRun;
    }

    for (const auto &op : output)
    {
        cout << op.d << ' ' << op.r << ' ' << op.c << ' ' << op.h << ' ' << op.w << '\n';
    }

    return 0;
}
