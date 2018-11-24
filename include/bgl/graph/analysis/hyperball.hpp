#pragma once
#include "bgl/graph/basic_graph.hpp"
#include "bgl/data_structure/hyperloglog_array.hpp"
#include <functional>
#include <cmath>

namespace bgl {
/// run simple HyperBall. to compute centrality, transpose graph in advance
/// @param g input unweighted graph
/// @param log2k specify the number of registers of HyperLogLog counter (5 <= log2k <= 20)
/// @param callback callback function (must be thread-safe).
///        arguments: current node (node_t), distance (node_t), estimated count (double)
/// @param threshold maximum number of iteration
/// @param num_threads number of threads (0: auto)
/// @see "HyperANF: Approximating the neighbourhood function of very large graphs on a budget"
///       (P. Boldi, M. Rosa and S. Vigna). In WWW'11.
///      "In-core computation of geometric centralities with HyperBall: A hundred billion nodes
///       and beyond" (P. Boldi and S. Vigna). In ICDMW'13.
void hyperball(const graph &g, int log2k, std::function<void(node_t, node_t, double)> callback,
               int threshold = 100, int num_threads = 0) {
  const node_t n = g.num_nodes();
  ASSERT(n > 0);

  hyperloglog_array curr_hll(n, log2k);
  std::vector<double> cache(n);

  g.for_each_node(fn(v) {
    curr_hll[v].insert(v);
    cache[v] = curr_hll[v].count();
    callback(v, 0, 1.0);
  }, num_threads);

  hyperloglog_array next_hll(curr_hll);

  std::vector<bool> curr_updated(n, true);
  std::vector<bool> next_updated;

  for (node_t d : irange(threshold)) {
    std::atomic<node_t> num_updated = 0;
    next_updated.assign(n, false);

    g.for_each_node(fn(u) {
      bool merged = false;
      for (node_t v : g.neighbors(u)) {
        if (curr_updated[v]) {
          merged = true;
          next_hll[u].merge(curr_hll[v]);
        }
      }
      if ((next_updated[u] = merged && curr_hll[u] != next_hll[u])) {
        ++num_updated;
        double count = next_hll[u].count();
        callback(u, d + 1, count - cache[u]);
        cache[u] = count;
      }
    }, num_threads);

    if (num_updated == 0) break;

    if (num_updated < n / 10) {
      g.for_each_node(fn(v) {
        if (next_updated[v]) {
          curr_hll[v] = next_hll[v];
        }
      }, num_threads);
    } else {
      curr_hll = next_hll;
    }

    std::swap(curr_updated, next_updated);
  }
}
} // namespace bgl
