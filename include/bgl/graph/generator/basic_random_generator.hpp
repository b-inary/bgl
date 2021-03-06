#pragma once
#include "bgl/graph/basic_graph.hpp"
#include "bgl/util/all.hpp"
#include <cstdint>
#include <limits>
#include <map>
#include <random>

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

/// [directed] generate a random graph by the configuration model with a given degree sequence.
/// note that configuration model can produce self loops and multiple edges.
/// @param g (directed) target graph that specifies a degree sequence
inline graph configuration(const graph &g) {
  std::vector<std::size_t> indegree(g.num_nodes());
  for (node_t v : g.nodes()) {
    for (node_t w : g.neighbors(v)) {
      ++indegree[w];
    }
  }

  std::vector<node_t> half_edges_in;
  std::vector<node_t> half_edges_out;
  for (node_t v : g.nodes()) {
    std::size_t indeg = indegree[v];
    std::size_t outdeg = g.outdegree(v);
    for (std::size_t i[[maybe_unused]] : irange(indeg)) {
      half_edges_in.push_back(v);
    }
    for (std::size_t i[[maybe_unused]] : irange(outdeg)) {
      half_edges_out.push_back(v);
    }
  }

  std::shuffle(half_edges_in.begin(), half_edges_in.end(), bgl_random);
  std::shuffle(half_edges_out.begin(), half_edges_out.end(), bgl_random);

  unweighted_adjacency_list adj(g.num_nodes());
  for (std::size_t i : irange(g.num_edges())) {
    node_t v = half_edges_out[i];
    node_t w = half_edges_in[i];
    adj[v].push_back(w);
  }

  return adj;
}

/// [directed] generate a random graph by 2D-configuration model (dK-2).
/// @see "Systematic topology analysis and generation using degree correlations"
///      (P. Mahadevan et al.). In SIGCOMM'06.
/// @param g (directed) target graph
/// @param bin_size bin size (log10 scale)
inline graph configuration_2d(const graph &g, int bin_size = 0) {
  std::vector<std::size_t> indegree(g.num_nodes());
  for (node_t v : g.nodes()) {
    for (node_t w : g.neighbors(v)) {
      ++indegree[w];
    }
  }

  std::map<std::size_t, std::vector<node_t>> half_edges_in;
  std::map<std::size_t, std::vector<node_t>> half_edges_out;
  for (node_t v : g.nodes()) {
    std::size_t indeg = indegree[v];
    std::size_t outdeg = g.outdegree(v);
    std::size_t in_bin = bin_size ? std::floor(std::log10(indeg) * bin_size) : indeg;
    std::size_t out_bin = bin_size ? std::floor(std::log10(outdeg) * bin_size) : outdeg;
    for (std::size_t i[[maybe_unused]] : irange(indeg)) {
      half_edges_in[in_bin].push_back(v);
    }
    for (std::size_t i[[maybe_unused]] : irange(outdeg)) {
      half_edges_out[out_bin].push_back(v);
    }
  }

  for (auto &p : half_edges_in) {
    std::shuffle(p.second.begin(), p.second.end(), bgl_random);
  }
  for (auto &p : half_edges_out) {
    std::shuffle(p.second.begin(), p.second.end(), bgl_random);
  }

  unweighted_adjacency_list adj(g.num_nodes());
  std::map<std::size_t, std::size_t> indeg_count;
  std::map<std::size_t, std::size_t> outdeg_count;

  for (node_t v : g.nodes()) {
    for (node_t w : g.neighbors(v)) {
      std::size_t out =
          bin_size ? std::floor(std::log10(g.outdegree(v)) * bin_size) : g.outdegree(v);
      std::size_t in = bin_size ? std::floor(std::log10(indegree[w]) * bin_size) : indegree[w];
      node_t new_v = half_edges_out[out][outdeg_count[out]++];
      node_t new_w = half_edges_in[in][indeg_count[in]++];
      adj[new_v].push_back(new_w);
    }
  }

  return adj;
}
}  // namespace bgl::gen
