#include "bgl/bgl.hpp"
using namespace bgl;

int main() {
  /* 1. 頂点の型 */

  // 頂点を表す型は |node_t| です．
  // 実際には using node_t = uint32_t として定義されています．
  node_t v = 0;


  /* 2. 辺の型 */

  // 重み無し辺，重み有り辺の型はそれぞれ |unweighted_edge_t|, |weighted_edge_t<WeightType>| です．

  // |unweighted_edge_t| は実際には |node_t| として定義されています．
  unweighted_edge_t e1 = 1;

  // |weighted_edge_t| は実際には |pair<node_t, WeightType>| として定義されています．
  weighted_edge_t<double> e2 = {1, 0.5};

  // 辺の行き先，重みを取得するにはフリー関数 |to|, |weight| を利用します．
  node_t u1 = to(e1);
  int w1 = weight(e1);  // 重み無し辺の重みは常に 1 (int) です．
  node_t u2 = to(e2);
  double w2 = weight(e2);


  /* 3. 辺リストと隣接リスト */

  // 辺リストは |unweighted_edge_list|, |weighted_edge_list<WeightType>| で表します．
  // 実際には vector<pair<node_t, EdgeType>> として定義されています．
  unweighted_edge_list es = {{0, 1}, {1, 2}};
  weighted_edge_list<double> wes = {{0, {1, 0.5}}, {1, {2, 1.5}}};

  // 隣接リストは |unweighted_adjacency_list|, |weighted_adjacency_list<WeightType>| で表します．
  // 実際には vector<vector<EdgeType>> として定義されています．
  unweighted_adjacency_list adj = {{1}, {2}, {}};
  weighted_adjacency_list<double> wadj = {{{1, 0.5}}, {{2, 1.5}}, {}};


  /* 4. bglグラフ */

  // bglグラフは辺リストまたは隣接リストから構成します．
  // 頂点数・辺数などは自動的に計算されます．

  // 重み無しグラフの型は |graph| です．
  graph g1 = es;
  graph g2 = adj;

  // 重み有りグラフの型は |wgraph<WeightType>| です．
  wgraph<double> wg1 = wes;
  wgraph<double> wg2 = wadj;


  /* 5. グラフジェネレータ */

  // いくつかのグラフジェネレータが |gen| 名前空間に実装されています．
  graph g3 = gen::complete(10);    // 10頂点の完全グラフ
  graph g4 = gen::grid(5, 5);      // 5×5のグリッドグラフ
}
