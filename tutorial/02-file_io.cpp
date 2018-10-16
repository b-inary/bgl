#include "bgl/bgl.hpp"
using namespace bgl;

int main(int argc, char **argv) {
  /* 1. tsvフォーマット */

  // bglでは，グラフのファイル入出力をいくつかの方法で行うことができます．
  // 1つ目の方法はタブ(スペース)区切りのtsvフォーマットを利用する方法です．
  // 標準の拡張子は ".tsv" です．

  // tsvでは，各行に (from_node) (to_node) をこの順に記録します．
  // 辺に重みがある場合は，この後ろに重みの情報も記録します．
  // bglでは先頭の文字が '#' の場合はコメント行として扱います．

  // tsvファイルを読み込むには，|read_graph_tsv()| 関数を利用します．
  // 関数を呼び出す際には，読み込む型を明示する必要があります．
  auto g1 = read_graph_tsv<graph>("dataset/karate.tsv");
  auto g2 = read_graph_tsv<wgraph<int>>("integer-weighted-graph.tsv");

  // tsvフォーマットでグラフを書き出すには，|write_graph_tsv()| を使います．
  write_graph_tsv("output-filename.tsv", g1);


  /* 2. bglフォーマット */

  // bglでより効率よく入出力を行えるバイナリフォーマットを利用することもできます．
  // 標準で拡張子 ".bgl" を利用することを想定しています．

  // bglファイルを読み込むには，|read_graph_binary()| 関数を利用します．
  auto g3 = read_graph_binary<graph>("bgl-file.bgl");

  // bglファイルを書き出すには，|write_graph_binary()| 関数を利用します．
  write_graph_binary("output-filename.bgl", g1);


  /* 3. フォルダ読み込み */

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
