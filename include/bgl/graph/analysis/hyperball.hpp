#pragma once
#include "bgl/graph/basic_graph.hpp"
#include "bgl/data_structure/hyperloglog_array.hpp"
#include <functional>
#include <cmath>

namespace bgl {
/// run simple HyperBall.
/// @param g input unweighted graph
/// @param log2k specify register number of HyperLogLog counter
/// @param callback callback function. arguments: current node, distance, count
/// @param threshold maximum number of iteration
/// @see "In-core computation of geometric centralities with hyperball:
///       A hundred billion nodes and beyond" (P. Boldi and S. Vigna). In ICDMW'13.
void hyperball(const graph &g, int log2k, std::function<void(node_t, node_t, double)> callback,
               int threshold = 100) {
  const node_t n = g.num_nodes();
  ASSERT(n > 0);

  hyperloglog_array cur_hll(n, log2k);
  for (node_t v : g.nodes()) {
    cur_hll[v].insert(v);
    callback(v, 0, 1.0);
  }
  hyperloglog_array nxt_hll(cur_hll);

  std::vector<double> cache(n, cur_hll[0].count());
  std::vector<bool> cur_updated(n, true);
  std::vector<bool> nxt_updated;

  for (node_t d : irange(threshold)) {
    bool loop = false;
    nxt_updated.assign(n, false);

    for (node_t u : g.nodes()) {
      bool merged = false;
      for (node_t v : g.neighbors(u)) {
        if (cur_updated[v]) {
          merged = true;
          nxt_hll[u].merge(cur_hll[v]);
        }
      }
      if ((nxt_updated[u] = merged) && cur_hll[u] != nxt_hll[u]) {
        loop = true;
        double count = nxt_hll[u].count();
        callback(u, d + 1, count - cache[u]);
        cache[u] = count;
      }
    }

    if (!loop) break;
    cur_hll = nxt_hll;
    cur_updated.swap(nxt_updated);
  }
}
} // namespace bgl
