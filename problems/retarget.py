#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""問題ファイルの「問題ごとに書き換える部分」を、同じカテゴリの別ライブラリに移植する。

  python3 retarget.py SRC.cpp KIND DST.cpp

KIND:
  貪欲系      : greedy / insertion / rolling
  焼きなまし系: hc / hckick / sa / msa / isa
  ビーム系    : beam / ibeam / chokudai
  モンテカルロ: mc / mcsh

移植先のアルゴリズム用パラメータ (温度・ビーム幅など) を用意し、
移植元の問題固有パラメータ (P_NB / EVAL_W / ROLLOUT_TH / ALPHA など) はそのまま引き継ぐ。
p1,p2,... の番号は振り直されるので、生成後にファイル先頭の対応表コメントを直すこと。
最大化/最小化も引き継ぐ。
"""
import io, os, re, sys

LIB = "/Users/hirokinagai/Desktop/AHC Library"
TPL = {"greedy": "greedy/1_greedy", "insertion": "greedy/2_insertion_greedy", "rolling": "greedy/3_rolling_horizon",
       "hc": "simulated annealing/1_hill_climbing", "hckick": "simulated annealing/2_hill_climbing_kick",
       "sa": "simulated annealing/3_simulated_annealing", "msa": "simulated annealing/4_multi_start_annealing",
       "isa": "simulated annealing/5_iterated_annealing",
       "beam": "beam search/1_beam_search", "ibeam": "beam search/2_incremental_beam_search",
       "chokudai": "beam search/3_chokudai_search",
       "mc": "monte carlo/1_monte_carlo", "mcsh": "monte carlo/2_successive_halving"}
# 移植先で必ず必要になるアルゴリズム用パラメータ (名前, 宣言, コメント)
ALGO = {
 "greedy": [], "insertion": [],
 "rolling": [("HORIZON", "int   HORIZON = -1;", "プレイアウトの手数 (-1 で最後まで)")],
 "hc": [], "hckick": [("KICK_STRENGTH", "int   KICK_STRENGTH = 3;", "キックの強さ"),
                      ("STAGNATION_LIMIT", "int   STAGNATION_LIMIT = 10000;", "キックまでの非改善回数")],
 "sa":  [("TEMP_START", "float TEMP_START = 50.0f;", "開始温度"), ("TEMP_END", "float TEMP_END = 1.0f;", "終了温度")],
 "msa": [("TEMP_START", "float TEMP_START = 50.0f;", "開始温度"), ("TEMP_END", "float TEMP_END = 1.0f;", "終了温度"),
         ("NUM_STARTS", "int   NUM_STARTS = 4;", "スタート回数")],
 "isa": [("TEMP_START", "float TEMP_START = 50.0f;", "開始温度"), ("TEMP_END", "float TEMP_END = 1.0f;", "終了温度"),
         ("NUM_ROUNDS", "int   NUM_ROUNDS = 4;", "繰り返し回数"), ("TEMP_DECAY", "float TEMP_DECAY = 0.7f;", "温度減衰率")],
 "beam": [("BEAM_WIDTH", "int   BEAM_WIDTH = 2000;", "ビーム幅")],
 "ibeam": [("BEAM_WIDTH", "int   BEAM_WIDTH = 2000;", "ビーム幅")],
 "chokudai": [("CHOKUDAI_WIDTH", "int   CHOKUDAI_WIDTH = 1;", "1 周で各深さから展開する数")],
 "mc":  [("TIME_ALPHA", "float TIME_ALPHA = 0.0f;", "ターンごとの時間配分"),
         ("MAX_SCENARIO", "int   MAX_SCENARIO = 1 << 30;", "シナリオ数の上限")],
 "mcsh":[("TIME_ALPHA", "float TIME_ALPHA = 0.0f;", "ターンごとの時間配分"),
         ("HALVE_RATE", "int   HALVE_RATE = 2;", "絞り込みの強さ"),
         ("MAX_SCENARIO", "int   MAX_SCENARIO = 1 << 30;", "シナリオ数の上限")],
}
PM, PE = "// ---- 調整パラメータ", "// =============================================================================\n// 4. スコアの最大化"
QM = "// #############################################################################\n// # 6. ■ ここから 問題ごとに書き換える部分 ■"
QE = "// # ■ 問題ごとに書き換える部分 ここまで ■\n// #############################################################################\n"

src_path, kind, dst_path = sys.argv[1], sys.argv[2], sys.argv[3]
src = io.open(src_path, encoding="utf-8").read()
tgt = io.open(os.path.join(LIB, TPL[kind], "main.cpp"), encoding="utf-8").read()
hdr = src[:src.index("#ifdef __x86_64__")]
prob = src[src.index(QM):src.index(QE) + len(QE)]

# 移植元のパラメータ宣言を順番に拾う
sp = src[src.index(PM):src.index(PE)]
sdecl = re.findall(r"^\s*((?:float|int)\s+(\w+)\s*=\s*[^;]+;)(.*)$", sp, re.M)
algo_names = [n for n, _, _ in ALGO[kind]]
lines, picks, used = [], [], set()
for n, d, c in ALGO[kind]:                       # 移植先のアルゴリズムパラメータ (移植元に同名があれば値を継承)
    for full, name, cm in sdecl:
        if name == n: d = full.strip(); c = re.sub(r"^.*?//\s*(?:p\d+:\s*)?", "", cm).strip() or c
    lines.append((n, d, c)); used.add(n)
for full, name, cm in sdecl:                     # 移植元の問題固有パラメータ
    if name in used: continue
    lines.append((name, full.strip(), re.sub(r"^.*?//\s*(?:p\d+:\s*)?", "", cm).strip()))
    used.add(name)
out_p = ["// ---- 調整パラメータ (p1, p2, ... が optimize.py のキーと対応) ----"]
k = 0
for n, d, c in lines:
    if n == "MAX_SCENARIO":
        out_p.append("%-34s // %s" % (d, c)); picks.append('    pick_env("MAX_SCENARIO", MAX_SCENARIO);')
    else:
        k += 1
        out_p.append("%-34s // p%d: %s" % (d, k, c)); picks.append('    pick_env("p%d", %s);' % (k, n))
out_p += ["", "void load_params() {"] + picks + ["}", ""]
tp = "\n".join(out_p) + "\n"

out = tgt[:tgt.index(PM)] + tp + tgt[tgt.index(PE):]
out = out[:out.index(QM)] + prob + out[out.index(QE) + len(QE):]
mx = ("constexpr bool MAXIMIZE = true;" in src) and ("// constexpr bool MAXIMIZE = true;" not in src)
res = []
for l in out.split("\n"):
    if "MAXIMIZE = true;" in l:    res.append("constexpr bool MAXIMIZE = true;        // ← スコア最大化" if mx else "// constexpr bool MAXIMIZE = true;     // ← スコア最大化")
    elif "MAXIMIZE = false;" in l: res.append("// constexpr bool MAXIMIZE = false;    // ← スコア最小化" if mx else "constexpr bool MAXIMIZE = false;       // ← スコア最小化")
    else: res.append(l)
out = "\n".join(res)
io.open(dst_path, "w", encoding="utf-8").write(hdr + out[out.index("#ifdef __x86_64__"):])
print("wrote", dst_path)
