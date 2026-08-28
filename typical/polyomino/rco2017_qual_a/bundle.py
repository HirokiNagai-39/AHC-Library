# -*- coding: utf-8 -*-
# main.cpp の #include "../polyomino.hpp" を展開して提出用の 1 ファイル submit.cpp を作る
#   python3 bundle.py
import os

here = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(here, "main.cpp"), encoding="utf-8").read()
lib = open(os.path.join(here, "..", "polyomino.hpp"), encoding="utf-8").read()
lib = lib.replace("#pragma once", "", 1)
out = src.replace('#include "../polyomino.hpp"', lib, 1)
path = os.path.join(here, "submit.cpp")
open(path, "w", encoding="utf-8").write(out)
print("wrote", path)
