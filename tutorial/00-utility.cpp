#include "bgl/bgl.hpp"
using namespace std;
using namespace bgl;

int main() {
  // bglにはプログラムを簡単に書くための強力なライブラリがいくつか用意されています．
  // bgl.hpp をインクルードするだけで，これらのライブラリを利用することができます．


  /* 1. 文字列フォーマット */

  // フォーマットにはfmtライブラリ (https://github.com/fmtlib/fmt) を用います．
  // pythonの |format()| などを利用したことがあれば簡単に書けるでしょう．

  // ストリームに出力するには |fmt::print()| を用います．
  fmt::print(cerr, "error: {}\n", "message");

  // 文字列を生成するには |fmt::format()| を用います．
  string s = fmt::format("string: {}, int: {}", "test", 42);

  // 複雑な書式指定も短く記述できます．
  fmt::print(cout, "{:>10.2f}\n", 3.141592);  // 右詰め，幅10文字，小数点以下2桁

  // また，pair, vector などの型や，ostream に << 演算子で出力できる型は
  // そのまま引数に指定できます．
  // (そのため独自に operator<< を定義するとコンフリクトすることがあります)
  fmt::print(cout, "{}\n", vector<pair<int, int>>{{1, 2}});


  /* 2. 簡単! ラムダマクロ */

  // C++11のラムダ式を (やや) 簡単に書けるようにするためのマクロが用意されています．
  // fn(a, b) は [&](const auto &a, const auto &b) に展開されます．(8引数まで対応)
  auto const42 = fn() { return 42; };
  auto add = fn(x, y) { return x + y; };


  /* 3. アサーションマクロ */

  // 動的なアサーションを行う |ASSERT()|, |ASSERT_MSG()| マクロが用意されています．
  // 条件が満たされなかった場合，まあまあ良い感じにエラーを出力して異常終了します．
  ASSERT(true);
  ASSERT_MSG(42 == 42, "check {}!", "failed");  // fmtによるエラーメッセージ指定

  // 失敗してもプログラムを続行したい場合は |EXPECT()|, |EXPECT_MSG()| を使います．
  EXPECT(false);   // warning を出力するがプログラムは終了しない


  /* 4. ログ出力マクロ */

  // 時刻とソースコードの位置を合わせて簡単にログが残せるマクロも用意されています．
  console_log("this is a {}.", "log");  // 標準エラー出力にログを書き出します

  // 実行時間を計測する |console_timer()|, |console_fn_timer| マクロもあります．
  auto lambda = fn() {
    console_fn_timer;   // ブロックの先頭で宣言すると，ブロック全体の実行時間を計測します
  };
  console_timer("some process", lambda);  // 第2引数で計測を行う関数を指定します

  // 出力ストリームを指定する場合はそれぞれ |write_log()|, |timer()|, |fn_timer()| です．
  write_log(std::cout, "write to standard output");


  /* 5. irange */

  // 整数の範囲を指定するforループを range-based for で記述できます．
  // vectorを生成したりする実装ではないので，動作も効率的です．
  for (int i : irange(10)) {}     //  [0, 10) の範囲をイテレーションする
  for (int i : irange(5, 10)) {}  // 開始位置の指定も可能 (この場合は [5, 10) を反復)


  /* 6. 乱数 */

  // 手軽に使えるグローバルな乱数生成機 |bgl_random| が用意されています．
  std::uniform_int_distribution<> dist(1, 10);
  int r = dist(bgl_random);

  // シードを与える場合は |seed()| を呼び出します．
  bgl_random.seed();    // デフォルトシードで初期化
  bgl_random.seed(5489);


  /* 7. ファイルパス操作 */

  // 本来ならばC++17の <filesystem> を利用したいのですが，現状では対応がまだ微妙です．
  // そこで，ミニ filesystem::path もどきが用意されています．
  path p = "src";
  p = p / "graph";  // パスの連結ができます (p == "src/graph" となる．p /= "graph" も同様)
  p.string();       // パスを文字列として得たい場合は |string()| を利用します
  p.extension();    // ファイル名や拡張子などが取得できます．詳細は src/util/file.hpp 参照

  // |path::find()|, |path::find_recursive()| でディレクトリ内のファイル検索もできます．
  // 検索で得られた結果は vector<path> として返ります．
  auto v = path::find_recursive(p, "*.hpp");  // ワイルドカードが利用できます
}
