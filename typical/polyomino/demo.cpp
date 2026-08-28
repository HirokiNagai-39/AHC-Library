// ポリオミノ生成ライブラリの動作確認 & 使い方デモ
//   g++-15 -std=c++20 -O2 -o demo demo.cpp && ./demo [MAXK]
#include "polyomino.hpp"

int main(int argc, char** argv) {
    int MAXK = argc > 1 ? atoi(argv[1]) : 10;

    // Wikipedia (ja) の個数表。[K][0]=両面, [K][1]=片面, [K][2]=有向
    const long long TBL[13][3] = {
        {0, 0, 0}, {1, 1, 1}, {1, 1, 2}, {2, 2, 6}, {5, 7, 19}, {12, 18, 63},
        {35, 60, 216}, {108, 196, 760}, {369, 704, 2725}, {1285, 2500, 9910},
        {4655, 9189, 36446}, {17073, 33896, 135268}, {63600, 126759, 505861},
    };

    printf("  K |     両面 FREE |    片面 ONE_SIDED |     有向 FIXED |   time[ms]\n");
    printf("----+---------------+-------------------+----------------+-----------\n");
    bool ok = true;
    for (int K = 1; K <= MAXK; K++) {
        auto t0 = chrono::steady_clock::now();
        long long n[3];
        n[0] = poly::generate(K, poly::FREE).size();
        n[1] = poly::generate(K, poly::ONE_SIDED).size();
        auto fx = poly::generate(K, poly::FIXED);
        n[2] = fx.size();
        double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
        bool good = (K > 12) || (n[0] == TBL[K][0] && n[1] == TBL[K][1] && n[2] == TBL[K][2]);
        ok &= good;
        printf("%3d | %13lld | %17lld | %14lld | %9.1f  %s\n", K, n[0], n[1], n[2], ms,
               good ? "OK" : "*** MISMATCH ***");

        // 「両面を 8 向きに展開すると有向全体になる」ことも確認
        auto ex = poly::expand_orientations(poly::generate(K, poly::FREE), poly::FREE);
        if (ex != fx) { printf("    *** expand_orientations(FREE) != FIXED ***\n"); ok = false; }
        auto ex2 = poly::expand_orientations(poly::generate(K, poly::ONE_SIDED), poly::ONE_SIDED);
        if (ex2 != fx) { printf("    *** expand_orientations(ONE_SIDED) != FIXED ***\n"); ok = false; }
        for (auto& s : fx) if (!poly::is_connected(s)) { printf("    *** disconnected ***\n"); ok = false; }
    }
    printf("\n[check] %s\n\n", ok ? "all counts match Wikipedia" : "FAILED");

    // ペントミノ(両面 12 種)を表示
    printf("=== ペントミノ 両面 FREE (%zu 種) ===\n", poly::generate(5, poly::FREE).size());
    printf("%s", poly::tile_ascii(poly::generate(5, poly::FREE), 12).c_str());

    // 片面だと L/J, S/Z のように裏返しが別種になる (12 -> 18)
    printf("=== ペントミノ 片面 ONE_SIDED (%zu 種) ===\n", poly::generate(5, poly::ONE_SIDED).size());
    printf("%s", poly::tile_ascii(poly::generate(5, poly::ONE_SIDED), 9).c_str());

    // 1 種類あたりの向きの数 (対称性が高い形ほど少ない)
    printf("=== テトロミノ(両面 5 種) の向きの数 ===\n");
    for (auto& s : poly::generate(4, poly::FREE)) {
        auto g = poly::to_ascii(s);
        printf("  %-14s 片面(回転のみ) %zu 向き / 両面(裏返しも) %zu 向き\n",
               g[0].c_str(), poly::orientations(s, poly::ONE_SIDED).size(),
               poly::orientations(s, poly::FREE).size());
        for (size_t i = 1; i < g.size(); i++) printf("  %s\n", g[i].c_str());
    }

    // ASCII から形を作る
    printf("\n=== from_ascii ===\n");
    auto t = poly::from_ascii({".#.", "###"});
    printf("T テトロミノ? cells=%zu connected=%d\n", t.size(), (int)poly::is_connected(t));
    printf("%s", poly::tile_ascii(poly::orientations(t, poly::FREE), 4).c_str());

    // 盤面への配置列挙 (for_each_placement)
    printf("\n=== for_each_placement: 6x6 盤に置ける通り数 ===\n");
    for (int K = 1; K <= 5; K++) {
        auto fx = poly::generate(K, poly::FIXED);
        long long cnt = 0, expect = 0;
        for (auto& s : fx) {
            poly::for_each_placement(6, 6, s, [&](const poly::Shape& t) {
                cnt++;
                for (auto& c : t) if (c.first < 0 || c.first >= 6 || c.second < 0 || c.second >= 6) ok = false;
                if ((int)t.size() != K) ok = false;
            });
            int h = poly::height(s), w = poly::width(s);
            if (h <= 6 && w <= 6) expect += (long long)(6 - h + 1) * (6 - w + 1);
        }
        printf("  K=%d : %lld 通り %s\n", K, cnt, cnt == expect ? "OK" : "*** NG ***");
        if (cnt != expect) ok = false;
    }

    printf("\n[check] %s\n", ok ? "ALL OK" : "FAILED");
    return ok ? 0 : 1;
}
