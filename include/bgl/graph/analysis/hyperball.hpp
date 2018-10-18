#pragma once
#include "bgl/graph/basic_graph.hpp"
#include "bgl/data_structure/hyperloglog_array.hpp"
#include <functional>
#include <cmath>

namespace bgl {
/// run simple HyperBall. when computing centrality, transpose graph in advance
/// @param g input unweighted graph
/// @param log2k specify the number of registers of HyperLogLog counter (5 <= log2k <= 20)
/// @param callback callback function. arguments: current node, distance, count
/// @param threshold maximum number of iteration
/// @see "In-core computation of geometric centralities with HyperBall:
///       A hundred billion nodes and beyond" (P. Boldi and S. Vigna). In ICDMW'13.
void hyperball(const graph &g, int log2k, std::function<void(node_t, node_t, double)> callback,
               int threshold = 1000) {
  const node_t n = g.num_nodes();
  ASSERT(n > 0);

  hyperloglog_array curr_hll(n, log2k);
  for (node_t v : g.nodes()) {
    curr_hll[v].insert(v);
    callback(v, 0, 1.0);
  }
  hyperloglog_array next_hll(curr_hll);

  std::vector<double> cache(n, curr_hll[0].count());
  std::vector<bool> curr_updated(n, true);
  std::vector<bool> next_updated;

  for (node_t d : irange(threshold)) {
    bool loop = false;
    next_updated.assign(n, false);

    for (node_t u : g.nodes()) {
      bool merged = false;
      for (node_t v : g.neighbors(u)) {
        if (curr_updated[v]) {
          merged = true;
          next_hll[u].merge(curr_hll[v]);
        }
      }
      if ((next_updated[u] = merged && curr_hll[u] != next_hll[u])) {
        loop = true;
        double count = next_hll[u].count();
        callback(u, d + 1, count - cache[u]);
        cache[u] = count;
      }
    }

    if (!loop) break;
    curr_hll = next_hll;
    std::swap(curr_updated, next_updated);
  }
}
} // namespace bgl
