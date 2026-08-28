# -*- coding: utf-8 -*-
# デモ(TSP)用のテストケース生成スクリプト
#   python3 gen.py 0 20      -> in/0000.txt .. in/0019.txt を作る
# 実際の AHC では公式の tools を使うので、これは動作確認用。
import os
import random
import sys

start = int(sys.argv[1]) if len(sys.argv) > 1 else 0
end = int(sys.argv[2]) if len(sys.argv) > 2 else 20
here = os.path.dirname(os.path.abspath(__file__))
os.makedirs(os.path.join(here, "in"), exist_ok=True)
os.makedirs(os.path.join(here, "out"), exist_ok=True)

for seed in range(start, end):
    rnd = random.Random(seed)
    n = rnd.randint(150, 300)
    lines = [str(n)]
    for _ in range(n):
        lines.append(f"{rnd.uniform(0, 1000):.3f} {rnd.uniform(0, 1000):.3f}")
    with open(os.path.join(here, "in", f"{seed:04d}.txt"), "w") as f:
        f.write("\n".join(lines) + "\n")
print(f"generated {end - start} cases in {os.path.join(here, 'in')}")
