#pragma once
#include "bgl/graph/basic_graph.hpp"
#include "bgl/util/all.hpp"

namespace bgl {
// compute clustering coefficient per degree up to |degree_threshold|: |g| must be undirected
std::vector<double> clustering_coefficient_per_degree(const graph &g, node_t degree_threshold) {
  std::vector<std::atomic<std::size_t>> count(degree_threshold + 1);
  std::vector<std::atomic<std::size_t>> num_triangles(degree_threshold + 1);

  g.for_each_node(fn(u, i [[maybe_unused]]) {
    std::size_t du = g.outdegree(u);
    if (du > degree_threshold) return;
    ++count[du];

    for (std::size_t i : irange(du)) {
      node_t v = g.neighbor(u, i);
      for (std::size_t j : irange(i + 1, du)) {
        node_t w = g.neighbor(u, j);
        if (g.is_adjacent(v, w)) {
          ++num_triangles[du];
        }
      }
    }
  });

  std::vector<double> result(degree_threshold + 1);
  for (node_t i : irange(degree_threshold + 1)) {
    if (num_triangles[i]) {
      double num_wedges = 0.5 * i * (i - 1) * count[i];
      result[i] = num_triangles[i] / num_wedges;
    } else {
      result[i] = 0.0;
    }
  }

  return result;
}
}  // namespace bgl
