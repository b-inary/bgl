#pragma once
#include "bgl/util/all.hpp"
#include "bgl/graph/basic_graph.hpp"
#include <limits>
#include <cstdint>

namespace bgl::gen {
//! [undirected] generate a complete graph (clique)
inline graph complete(node_t num_nodes) {
  ASSERT(num_nodes > 0);
  unweighted_adjacency_list adj(num_nodes);
  for (node_t u : irange(num_nodes)) {
    for (node_t v : irange(u)) {
      adj[u].push_back(v);
      adj[v].push_back(u);
    }
  }
  return adj;
}

//! [directed] generate a directed complete bipartite graph
inline graph dir_complete_bipartite(node_t num_left, node_t num_right) {
  ASSERT(num_left + num_right > 0);
  unweighted_adjacency_list adj(num_left + num_right);
  for (node_t u : irange(num_left)) {
    for (node_t v : irange(num_right)) {
      adj[u].push_back(v + num_left);
    }
  }
  return adj;
}

//! [undirected] generate a complete bipartite graph
inline graph complete_bipartite(node_t num_left, node_t num_right) {
  return dir_complete_bipartite(num_left, num_right).make_undirected();
}

//! [undirected] generate a star graph
inline graph star(node_t num_nodes) {
  ASSERT(num_nodes > 0);
  unweighted_adjacency_list adj(num_nodes);
  for (node_t v : irange(1u, num_nodes)) {
    adj[0].push_back(v);
    adj[v].push_back(0);
  }
  return adj;
}

//! [undirected] generate a 3D-grid graph
inline graph grid_3d(node_t num_x, node_t num_y, node_t num_z) {
  ASSERT(num_x > 0 && num_y > 0 && num_z > 0);
  ASSERT_MSG(std::uint64_t(num_x) * num_y * num_z <= std::numeric_limits<node_t>::max(),
              "graph too large");

  unweighted_adjacency_list adj(num_x * num_y * num_z);
  auto idx = lambda(x, y, z) { return x + num_x * (y + num_y * z); };
  for (node_t z : irange(num_z)) {
    for (node_t y : irange(num_y)) {
      for (node_t x : irange(num_x)) {
        if (x + 1 < num_x) {
          adj[idx(x, y, z)].push_back(idx(x + 1, y, z));
          adj[idx(x + 1, y, z)].push_back(idx(x, y, z));
        }
        if (y + 1 < num_y) {
          adj[idx(x, y, z)].push_back(idx(x, y + 1, z));
          adj[idx(x, y + 1, z)].push_back(idx(x, y, z));
        }
        if (z + 1 < num_z) {
          adj[idx(x, y, z)].push_back(idx(x, y, z + 1));
          adj[idx(x, y, z + 1)].push_back(idx(x, y, z));
        }
      }
    }
  }

  return adj;
}

//! [undirected] generate a 2D-grid graph
inline graph grid(node_t num_cols, node_t num_rows) {
  ASSERT(num_cols > 0 && num_rows > 0);
  return grid_3d(num_cols, num_rows, 1);
}

//! [undirected] generate a path graph
inline graph path(node_t num_nodes) {
  ASSERT(num_nodes > 0);
  return grid_3d(num_nodes, 1, 1);
}

//! [directed] generate a directed cycle graph
inline graph dir_cycle(node_t num_nodes) {
  ASSERT(num_nodes > 0);
  unweighted_adjacency_list adj(num_nodes);
  for (node_t v : irange(num_nodes - 1)) {
    adj[v].push_back(v + 1);
  }
  if (num_nodes > 1) {
    adj[num_nodes - 1].push_back(0);
  }
  return adj;
}

//! [undirected] generate a cycle graph
inline graph cycle(node_t num_nodes) {
  ASSERT(num_nodes > 0);
  return dir_cycle(num_nodes).make_undirected();
}
} // namespace bgl::gen
