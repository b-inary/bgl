#include "bgl/bgl.hpp"
using namespace bgl;

int main(int argc, char **argv) {
  /* 1. ファイルの読み込み */

  // bglでは，2種類のファイルフォーマットをサポートしています．

  //  - tsvフォーマット:
  //      各行に (from_node) (to_node) をこの順に記録します．
  //      辺に重みがある場合は，この後ろに重みの情報も記録します．
  //      bglでは先頭の文字が '#' の場合はコメント行として扱います．
  //  - bglフォーマット:
  //      bglでより効率よく入出力を行えるバイナリフォーマットです．

  // さらに，これらのデータを Zstandard (zstd) 形式で圧縮したファイルにも対応しています．
  // (なお，読み込まれたグラフはメモリ上に展開されるため，ファイルサイズが小さくても
  //  メモリ使用量が非常に大きくなる場合があるので注意してください)

  // ファイル形式は拡張子によって判別されます．(.tsv / .bgl / .tsv.zst / .bgl.zst)

  // ファイルを読み込むには，|read_graph()| 関数を利用します．
  // 関数を呼び出す際には読み込む型を明示する必要があります．
  auto g1 = read_graph<graph>("dataset/karate.tsv");
  auto g2 = read_graph<wgraph<int>>("integer-weighted-graph.tsv");
  auto g3 = read_graph<graph>("bglformat.bgl");

  // 読み込む形式を明示的に指定することもできます．
  // (関数名は read_graph_(tsv|binary)(_zstd)?)
  auto g4 = read_graph_binary_zstd<graph>("compressed.bgl.zst");


  /* 2. ファイルの書き込み */

  // tsvフォーマットでグラフを書き出すには，|write_graph_tsv()| を使います．
  write_graph_tsv("output-filename.tsv", g1);

  // bglファイルを書き出すには，|write_graph_binary()| 関数を利用します．
  write_graph_binary("output-filename.bgl", g1);

  // なおbglではzstd形式でファイルを圧縮しての書き込みには対応していません．
  // 圧縮を行う場合は外部のコマンドラインツールなどを利用してください．


  /* 3. フォルダの読み込み */

  // フォルダ内のすべてのグラフそれぞれに処理を行いたいこともあるでしょう．
  // そのような際に簡単にフォルダをスキャンできるクラスが用意されています．

  // |graph_folder_iterator| の第1引数にはフォルダ名を指定します．
  // 第2引数を true に設定すると，フォルダ内のフォルダも再帰的にスキャンします．
  for (auto [g, p] : graph_folder_iterator<graph>("some-folder", true)) {
    // 変数 |g| に |graph| 型のグラフが，変数 |p| にグラフのパスが入っています
  }


  /* 4. CUIアプリケーション */

  // 読み込むファイルなどをコマンドラインで指定したいという場合も簡単です!
  // |bgl_app| クラスの変数を作り，|BGL_PARSE()| を呼び出すだけです．
  bgl_app app("my description");
  BGL_PARSE(app, argc, argv);

  // あとは，メンバ関数 |graph_iterator()| を呼び出すと，グラフとパスにアクセスできます．
  for (auto [g, p] : app.graph_iterator<graph>()) {}

  // なお，CLI11ライブラリ (https://github.com/CLIUtils/CLI11) を継承して実装しているため，
  // CLI11ライブラリと同様にオプションの追加なども行えます．
  bool my_flag;
  int my_int;
  app.add_flag("--my-flag", my_flag, "description of flag");
  app.add_option("--my-int", my_int, "description of int");
}
