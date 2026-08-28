# -*- coding: utf-8 -*-
# RCO 2017 予選 A (Multiple Pieces) のテストケース生成
#   python3 gen.py 0 20   -> in/0000.txt .. in/0019.txt
#   入力形式: H W K / 長さ W の数字文字列 H 行 (各マス 0..9 の一様乱数)
import os
import random
import sys

start = int(sys.argv[1]) if len(sys.argv) > 1 else 0
end = int(sys.argv[2]) if len(sys.argv) > 2 else 20
H, W, K = 50, 50, 8
here = os.path.dirname(os.path.abspath(__file__))
os.makedirs(os.path.join(here, "in"), exist_ok=True)
os.makedirs(os.path.join(here, "out"), exist_ok=True)

for seed in range(start, end):
    rnd = random.Random(seed)
    lines = [f"{H} {W} {K}"]
    for _ in range(H):
        lines.append("".join(str(rnd.randint(0, 9)) for _ in range(W)))
    with open(os.path.join(here, "in", f"{seed:04d}.txt"), "w") as f:
        f.write("\n".join(lines) + "\n")
print(f"generated {end - start} cases in {os.path.join(here, 'in')}")
