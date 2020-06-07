#include "bgl/bgl.hpp"
#include <queue>
using namespace std;
using namespace bgl;

int main() {
  graph g = gen::grid(3, 3);

  /* 1. 基本的な情報 */

  // グラフの頂点数は |num_nodes()|, 辺数は |num_edges()| で取得できます．
  node_t num_v = g.num_nodes();
  size_t num_e = g.num_edges();

  // 辺の型および重みの型は |edge_type|, |weight_type| で取得できます．
  graph::edge_type e;
  graph::weight_type w;

  // |pretty_print()| でグラフの情報を出力します．
  g.pretty_print();   // デフォルトでは標準エラー出力に書き出す (引数でストリームの指定が可能)


  /* 2. グラフアクセス */

  // グラフの各頂点にアクセスするには |nodes()| を用いて range-based for を使います．
  for (node_t v : g.nodes()) {

    // 頂点 |v| から出る辺を走査するには，|edges()| 関数を用います．
    for (auto &e : g.edges(v)) {}

    // C++17の記法を利用することで，重み有りグラフの辺の走査は次のようにも書けます．
    // for (auto [node, weight] : g.edges(v)) {}

    // 辺の重みに興味がない場合は，|neighbors()| を使うことで重み無し/有りで共通に書けます．
    for (node_t u : g.neighbors(v)) {}

    // 頂点 |v| の出次数は |outdegree()| で取得できます．
    size_t d = g.outdegree(v);

    // |edge()|, |neighbor()| 関数でインデックスを指定した辺/隣接頂点の取得も可能ですが，
    // ランダムアクセスがしたいというような場合以外は用いない方が良いでしょう．
    auto e = g.edge(v, 0);  // 頂点 |v| を始点とする最初の辺を取得
  }

  // |for_each_node()| を用いると各頂点について簡単に並列処理を行うことができます．
  // 引数の関数はスレッドセーフである必要があります．(グラフの動的変更などは基本的に不可)
  // 適切に atomic 型や mutex などを利用してください．
  g.for_each_node(fn(v, i) { /* 頂点 |v| に関する処理 (|i| はスレッド番号) */ });

  // |is_adjacent()| で頂点が隣接しているかどうかを確認できます．
  bool b = g.is_adjacent(0, 1);

  // |get_weight()| で辺の重みを取得できます．(辺が複数ある場合は最小の重み)
  // 頂点が隣接していない場合は nullopt が返ります．
  optional<int> w_opt = g.get_weight(0, 1);


  /* 3. グラフの簡単な変換 */

  // |simplify()| でグラフの自己ループ，多重辺を削除できます．(破壊的操作; 以下の関数も同様)
  // 引数に true を指定すると，重みの異なる多重辺は保持されます．
  g.simplify();

  // |transpose()| でグラフの転置が行えます．
  g.transpose();

  // |make_undirected()| で有向グラフを無向化できます．
  // (実装上の副作用として，同じ重みの多重辺がある場合は削除されます)
  g.make_undirected();

  // |remove_isolated_nodes()| で孤立点を削除できます．頂点のIDが変わる可能性があります．
  g.remove_isolated_nodes();


  /* 4. グラフの動的な操作 */

  // bglでは動的な辺の追加・削除が一応可能になっています．
  // しかし，内部ではソートされた隣接リストでデータを保持しているため，動的操作はあまり軽くなく，
  // 使用は極力避けることを想定しています．
  g.add_edge(0, 2);
  g.remove_edge(0, 2);  // 頂点 0 と頂点 2 を繋ぐ辺が (多重辺を含めて) 全て削除される


  /* 5. アルゴリズムの実装例: 幅優先探索 */

  queue<node_t> que;
  vector<bool> visited(g.num_nodes());

  // 頂点0から幅優先探索する
  que.push(0);
  visited[0] = true;

  while (!que.empty()) {
    node_t v = que.front();
    que.pop();
    for (node_t u : g.neighbors(v)) {
      if (visited[u]) continue;
      fmt::print("visit: {}\n", u);   // 頂点 u に到達
      que.push(u);
      visited[u] = true;
    }
  }


  /* 6. グラフ上の探索 */

  // 幅優先探索の実装例を挙げましたが，標準で幅優先探索・Dijkstraアルゴリズムが利用できます．

  // 頂点 0 から近い頂点を順に訪問します．
  // ラムダ式が false を返すと，その頂点からの探索を打ち切ります．
  visit_by_distance(g, 0, fn(v, w) {
    fmt::print("visit: {} / distance: {}\n", v, w);
    return true;
  });

  // ただし，|visit_by_distance| 関数は初期化に O(n) 時間を要します．
  // 狭い範囲の探索を繰り返す場合，|visitor_by_distance| クラスを利用することで
  // 再初期化のコストを抑えられます．
  visitor_by_distance<graph> visitor(g);
  visitor.visit(0, fn(v, w) { return true; });

  // 単一始点最短距離は |single_source_distance()| 関数で求められます．
  vector<int> distances = single_source_distance(g, 0);
}
