#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""問題ファイルをコンパイルして走らせ、スコアを表示する。

  python3 eval.py FILE.cpp                       # seed 0,1,2 で実行
  python3 eval.py FILE.cpp --seeds 0,1,2,3,4     # seed を指定
  python3 eval.py FILE.cpp --env p1=2.0,p2=0.3   # パラメータを環境変数で上書き
  python3 eval.py --ab BASE.cpp CAND.cpp --seeds 0,1,2   # 2 つを seed ごとに交互実行して比較
                                                          # (並列負荷のブレを打ち消せるので推奨)
出力の SUM を比べる。スコアの向き (大きいほど良い/小さいほど良い) は
ファイル先頭の【スコア】を読むこと。
"""
import argparse, os, re, subprocess, sys, tempfile

def build(src):
    exe = tempfile.mktemp(prefix="ahceval_")
    r = subprocess.run(["g++", "-std=c++20", "-O2", "-o", exe, src], capture_output=True)
    if r.returncode != 0:
        print("[COMPILE FAIL] %s\n%s" % (src, r.stderr.decode()[-3000:]))
        sys.exit(1)
    return exe

def run1(exe, seed, env):
    e = dict(os.environ); e["SEED"] = str(seed); e.update(env)
    r = subprocess.run([exe], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                       stderr=subprocess.PIPE, env=e, timeout=180)
    err = r.stderr.decode()
    if "[error]" in err:
        print("  !! 不正な出力: " + [l for l in err.split("\n") if "[error]" in l][0])
        return None
    m = re.search(r"Score = (-?\d+)", err)
    return int(m.group(1)) if m else None

def parse_env(s):
    d = {}
    if s:
        for kv in s.split(","):
            k, v = kv.split("=", 1); d[k.strip()] = v.strip()
    return d

ap = argparse.ArgumentParser()
ap.add_argument("files", nargs="+")
ap.add_argument("--ab", action="store_true")
ap.add_argument("--seeds", default="0,1,2")
ap.add_argument("--env", default="")
ap.add_argument("--env2", default="")
a = ap.parse_args()
seeds = [int(x) for x in a.seeds.split(",")]

if a.ab:
    if len(a.files) != 2: sys.exit("--ab には 2 ファイル指定してください")
    e1, e2 = build(a.files[0]), build(a.files[1])
    v1 = parse_env(a.env); v2 = parse_env(a.env2 or a.env)
    s1 = s2 = 0
    for sd in seeds:                      # seed ごとに交互に走らせて負荷の偏りを消す
        x = run1(e1, sd, v1); y = run1(e2, sd, v2)
        if x is None or y is None: sys.exit("実行に失敗しました")
        s1 += x; s2 += y
        print("seed=%-3d  A=%-12d B=%-12d  (B-A=%+d)" % (sd, x, y, y - x))
    print("SUM   A=%d  B=%d  (B-A=%+d, %+.2f%%)" % (s1, s2, s2 - s1, 100.0 * (s2 - s1) / max(1, abs(s1))))
    os.remove(e1); os.remove(e2)
else:
    exe = build(a.files[0]); env = parse_env(a.env); tot = 0
    for sd in seeds:
        v = run1(exe, sd, env)
        if v is None: sys.exit("実行に失敗しました")
        tot += v
        print("seed=%-3d score=%d" % (sd, v))
    print("SUM=%d" % tot)
    os.remove(exe)
