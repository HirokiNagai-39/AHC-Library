# ポリオミノ生成ライブラリ

K 個の正方形からなるポリオミノを、**有向 / 片面 / 両面**を切り替えて全列挙するライブラリです。
定義と個数は [ja.wikipedia.org/wiki/ポリオミノ](https://ja.wikipedia.org/wiki/%E3%83%9D%E3%83%AA%E3%82%AA%E3%83%9F%E3%83%8E) に合わせてあります。

```
typical/polyomino/
├── polyomino.hpp          ライブラリ本体（ヘッダ 1 枚・namespace poly）
├── demo.cpp               動作確認（個数表との照合・ASCII 表示・配置列挙）
└── rco2017_qual_a/        例題: RCO 2017 予選 A - Multiple Pieces
    ├── main.cpp           1.生成 → 2.配置全列挙 → 3.スコア降順に貪欲
    ├── bundle.py          #include を展開して提出用 submit.cpp を作る
    ├── pahcer_config.toml
    └── tools/gen.py, tools/score.py
```

```bash
cd typical/polyomino
g++-15 -std=c++20 -O2 -o demo demo.cpp && ./demo 10
```

---

## 1. 3 つの数え方

| 定数 | 和名 | 同一視するもの | 盤面に置くときの向きの数 |
|---|---|---|---|
| `poly::FIXED` | 有向 | 平行移動のみ | 1 |
| `poly::ONE_SIDED` | 片面 | 平行移動 + 回転 | 最大 4（回転のみ、裏返し禁止） |
| `poly::FREE` | 両面 | 平行移動 + 回転 + 裏返し | 最大 8 |

`demo.cpp` が Wikipedia の個数表と一致することを毎回確認します（実測、K=10 まで 0.07 秒）。

| K | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| 両面 | 1 | 1 | 2 | 5 | 12 | 35 | 108 | 369 | 1285 | 4655 |
| 片面 | 1 | 1 | 2 | 7 | 18 | 60 | 196 | 704 | 2500 | 9189 |
| 有向 | 1 | 2 | 6 | 19 | 63 | 216 | 760 | 2725 | 9910 | 36446 |

生成は **Redelmeier のアルゴリズム**（有向ポリオミノを重複なく直接列挙）＋正規形での重複除去。
実用域は **K ≤ 12**（有向 505861 個）くらいまでです。

---

## 2. API

```cpp
#include "polyomino.hpp"

using Cell  = pair<int,int>;   // (y, x)
using Shape = vector<Cell>;    // 正規化済み: 昇順ソート & min(y)=min(x)=0

vector<Shape> poly::generate(int K, Symmetry sym);        // K マスを全列挙（辞書順）
vector<vector<Shape>> poly::generate_upto(int K, Symmetry sym);

vector<Shape> poly::orientations(const Shape&, Symmetry); // その形が取れる向きを重複なく
Shape         poly::canonical  (const Shape&, Symmetry);  // 同値類の代表元
Shape         poly::transform  (const Shape&, int t);     // D4 の 8 変換 (t=0..7)
Shape         poly::normalize  (Shape);
int           poly::height(const Shape&), poly::width(const Shape&);
bool          poly::is_connected(const Shape&);

// 盤面 H x W に平行移動して置ける全パターン（回転・裏返しは先に展開しておく）
template<class F> void poly::for_each_placement(int H, int W, const Shape&, F cb);
vector<Shape> poly::expand_orientations(const vector<Shape>&, Symmetry);

// デバッグ表示
vector<string> poly::to_ascii(const Shape&);
Shape          poly::from_ascii(const vector<string>&);   // {".#.", "###"} → T テトロミノ
string         poly::tile_ascii(const vector<Shape>&, int cols = 10);
```

### 使い分けの注意

**「全種類のポリオミノを盤面に置く」問題では、どのモードで生成しても結果は同じ**になります。
両面 369 種を 8 向きに展開しても、片面 704 種を 4 向きに展開しても、有向 2725 種そのものと
一致するからです（`demo.cpp` で毎回検証しています）。モードが効いてくるのは次のような場合です。

- **種類数が問題になるとき**（「各種類を 1 個ずつ使える」など）→ 生成モードがそのまま種類数
- **与えられた 1 つの形の向きを列挙するとき** → `orientations(s, ONE_SIDED)` なら裏返し禁止、
  `orientations(s, FREE)` なら裏返し可
- 対称性の高い形（O テトロミノは 1 向き、I は 2 向き）を重複なく扱いたいとき

---

## 3. 例題: RCO 2017 予選 A - Multiple Pieces

<https://atcoder.jp/contests/rco-contest-2017-qual/tasks/rco_contest_2017_qual_a>

50×50 のグリッドの各マスに 0〜9 の数字。ちょうど 8 マスの連結領域（ピース）を、マスを共有しない
ように好きなだけ作る。ピースのスコアはその 8 個の数字の**積**。合計の最大化（提出得点は合計 ÷ 10000 の切り上げ）。

```bash
cd rco2017_qual_a
python3 tools/gen.py 0 10                  # tools/in/0000.txt .. 0009.txt
g++-15 -std=c++20 -O2 -o a.out main.cpp
./a.out < tools/in/0000.txt > tools/out/0000.txt
python3 tools/score.py tools/in/0000.txt tools/out/0000.txt   # 独立採点＋制約チェック
python3 bundle.py                          # 提出用 submit.cpp（1 ファイル）
```

### 解法（指定どおりの 3 ステップ）

1. **ポリオミノを生成する** — `poly::generate(8, FREE)` で両面 369 種、
   `poly::expand_orientations` で盤面に置ける 2725 向きに展開。
2. **置き方を全列挙する** — 2725 向き × 平行移動 = **6,005,662 通り**。
   盤面からはみ出さず、**0 を含まない**ものだけ残して **2,374,777 通り**（積が 0 のピースは
   完全に無駄なので捨ててよい）。各配置のスコア＝ 8 マスの積を持たせる。
3. **スコアの大きい順にソートして、置けるなら置く** を最後まで繰り返す。

メモリは配置ごとに `{積(uint32), マス列への添字(uint32)}` の 8 バイトだけ持ち、マス列は
`uint16` の平坦配列に押し込んでいます（ピーク 68 MB / 制限 1024 MB）。

### 実測（seed 0〜9、10 ケース）

| seed | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 合計 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 得点 | 80422 | 100298 | 96769 | 90507 | 89628 | 95092 | 88873 | 78236 | 89430 | 88606 | **897861** |

1 ケース **約 140 ms**（生成 4 ms / 列挙 41 ms / ソート 62 ms、制限 10 秒）。
置けたピースは 201〜213 個（上限は 2500 ÷ 8 = 312 個）。
`[check]` で「重複マスなし・ちょうど 8 マス・連結・積の合計の一致」を毎回検証しています。

### パラメータ

`p1` … 生成モード（0 = 有向 / 1 = 片面 / **2 = 両面**、既定 2）。

上で書いたとおりこの問題ではどれでもスコアが同じ（実測でも 3 モードとも 80422）なので、
**この例題に optuna で調整すべきパラメータはありません**。`optimize.py` は置いていません。
`pahcer_config.toml` は 10 ケースの一括計測用に置いてあります。

### この貪欲の限界

スコア降順に置くだけなので、「高得点ピースが場所を食い合う」「終盤に 9 が孤立して使えない」
といった無駄が残ります。伸ばすなら次のような改良が考えられます（未実装）。

- 置いた後に空きマスの分断を減らす向きを優先する（評価値に空きマス連結性の項を足す）
- ランダムに一部のピースを外して置き直す破壊再構築（この規模なら 10 秒で数百回まわせる）
