# -*- coding: utf-8 -*-
# 出力の検証と採点   python3 tools/score.py in/0000.txt out/0000.txt
import sys
inp, outp = sys.argv[1], sys.argv[2]
it = open(inp).read().split()
H, W, K = int(it[0]), int(it[1]), int(it[2])
grid = it[3:3+H]
tok = open(outp).read().split()
C = int(tok[0]); p = 1
used = set(); total = 0
for c in range(C):
    cells = []
    for k in range(K):
        y, x = int(tok[p]) - 1, int(tok[p+1]) - 1; p += 2
        assert 0 <= y < H and 0 <= x < W, "out of grid"
        assert (y, x) not in used, "cell reused"
        used.add((y, x)); cells.append((y, x))
    s = set(cells); assert len(s) == K, "duplicate cell in piece"
    st = [cells[0]]; vis = {cells[0]}
    while st:
        y, x = st.pop()
        for dy, dx in ((1,0),(-1,0),(0,1),(0,-1)):
            n = (y+dy, x+dx)
            if n in s and n not in vis: vis.add(n); st.append(n)
    assert len(vis) == K, "piece not connected"
    pr = 1
    for y, x in cells: pr *= int(grid[y][x])
    total += pr
print(f"C={C} sum={total} Score={-(-total//10000)}")
