#pragma once
#include "bgl/util/all.hpp"
#include "bgl/graph/basic_graph.hpp"
#include <map>
#include <limits>
#include <random>
#include <cstdint>

namespace bgl::gen {
/// [undirected] generate a random graph by the Erdős-Rényi model
/// @see "Efficient generation of large random networks" (V. Batagelj & U. Brandes).
///      Physical Review E, 2005.
inline graph erdos_renyi(node_t num_nodes, double average_degree) {
  double p = std::min(average_degree / (num_nodes - 1), 1.0);
  if (num_nodes <= 1 || is_zero(p)) return {num_nodes, {}};

  double logp = std::log(1 - p);
  unweighted_adjacency_list adj(num_nodes);
  std::uniform_real_distribution<> dist(0.0, 1.0);
  int64_t v = 1, w = -1;

  while (true) {
    double r = dist(bgl_random);
    // skip trials of geometrically distributed number
    w += 1 + std::floor(std::log(1 - r) / logp);
    while (w >= v) {
      w -= v;
      if (++v == num_nodes) return adj;
    }
    adj[v].push_back(w);
    adj[w].push_back(v);
  }
}

/// [undirected] generate a random graph by the configuration model with a given degree sequence.
/// note that configuration model can produce self loops and multiple edges.
/// @param g target graph (must be undirected without loops)
inline graph configuration(const graph &g) {
  std::vector<node_t> half_edges;
  for (node_t v : g.nodes()) {
    std::size_t deg = g.outdegree(v);
    for (std::size_t i [[maybe_unused]] : irange(deg)) {
      half_edges.push_back(v);
    }
  }

  std::shuffle(half_edges.begin(), half_edges.end(), bgl_random);

  unweighted_adjacency_list adj(g.num_nodes());
  const std::size_t num_edges = half_edges.size() / 2;
  for (std::size_t i : irange(num_edges)) {
    node_t v = half_edges[i * 2];
    node_t w = half_edges[i * 2 + 1];
    adj[v].push_back(w);
    adj[w].push_back(v);
  }

  return adj;
}

/// [undirected] generate a random graph by 2D-configuration model (dK-2).
/// @see "Systematic topology analysis and generation using degree correlations"
///      (P. Mahadevan et al.). In SIGCOMM'06.
/// @param g target graph (must be undirected without loops)
inline graph configuration_2d(const graph &g) {
  std::map<std::size_t, std::vector<node_t>> half_edges;
  for (node_t v : g.nodes()) {
    std::size_t deg = g.outdegree(v);
    for (std::size_t i [[maybe_unused]] : irange(deg)) {
      half_edges[deg].push_back(v);
    }
  }

  for (auto &p : half_edges) {
    std::shuffle(p.second.begin(), p.second.end(), bgl_random);
  }

  unweighted_adjacency_list adj(g.num_nodes());
  std::map<std::size_t, std::size_t> deg_count;
  for (node_t v : g.nodes()) {
    for (node_t w : g.neighbors(v)) {
      if (v > w) continue;
      std::size_t dv = g.outdegree(v);
      std::size_t dw = g.outdegree(w);
      node_t new_v = half_edges[dv][deg_count[dv]++];
      node_t new_w = half_edges[dw][deg_count[dw]++];
      adj[new_v].push_back(new_w);
      adj[new_w].push_back(new_v);
    }
  }

  return adj;
}
} // namespace bgl::gen
