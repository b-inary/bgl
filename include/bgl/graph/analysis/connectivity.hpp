#pragma once
#include "bgl/graph/visitor.hpp"
#include "bgl/data_structure/union_find.hpp"
#include <unordered_map>
#include <stack>

namespace bgl {
/// decompose graph into WCCs: nodes in the same components share the same ID
/// @return pair of the number of components and list of component IDs
template <typename GraphType>
std::pair<node_t, std::vector<node_t>> weakly_connected_components(const GraphType &g) {
  const node_t n = g.num_nodes();
  union_find<node_t> uf(n);
  std::vector<bool> visited(n, false);
  visitor_by_distance<GraphType> visitor(g);

  for (node_t v : g.nodes()) {
    visitor.visit(v, lambda(w, d [[maybe_unused]]) {
      uf.unite(v, w);
      if (visited[w]) return false;
      visited[w] = true;
      return true;
    });
  }

  node_t num_components = 0;
  std::vector<node_t> ids = uf.components();
  std::unordered_map<node_t, node_t> id_map;
  for (node_t &id : ids) {
    if (id_map.count(id) == 0) {
      id_map[id] = num_components++;
    }
    id = id_map[id];
  }

  return {num_components, ids};
}

/// [destructive] extract largest WCC
template <typename GraphType>
GraphType& extract_largest_wcc(GraphType &g) {
  auto [num_components, ids] = weakly_connected_components(g);
  std::vector<node_t> num_nodes(num_components);
  for (node_t id : ids) {
    num_nodes[id]++;
  }

  std::vector<bool> filter_list(g.num_nodes());
  node_t max_id = std::max_element(num_nodes.begin(), num_nodes.end()) - num_nodes.begin();
  for (node_t v : g.nodes()) {
    filter_list[v] = ids[v] == max_id;
  }

  return g.filter_nodes(filter_list);
}

/// determine if graph is (weakly) connected
template <typename GraphType>
bool is_connected(const GraphType &g) {
  return weakly_connected_components(g).first == 1;
}

/// decompose graph into SCCs: use Tarjan's SCC algorithm
/// @return pair of the number of components and list of component IDs
template <typename GraphType>
std::pair<node_t, std::vector<node_t>> strongly_connected_components(const GraphType &g) {
  const node_t n = g.num_nodes();
  node_t index = 0;
  node_t num_visited = 0;
  std::vector<bool> visited(n, false);
  std::vector<bool> on_stack(n, false);
  std::vector<node_t> ids(n, 100);
  std::vector<node_t> order(n);
  std::vector<node_t> lowlink(n);
  std::stack<node_t> node_stack;
  std::stack<std::pair<node_t, node_t>> dfs_stack;

  for (node_t u : g.nodes()) {
    if (visited[u]) continue;
    dfs_stack.emplace(u, 0);

    // to prevent stack overflow, do dfs using stack data structure
    while (!dfs_stack.empty()) {
      node_t v = dfs_stack.top().first;
      node_t i = dfs_stack.top().second;
      dfs_stack.pop();

      if (i == 0) {
        visited[v] = true;
        order[v] = lowlink[v] = num_visited++;
        node_stack.push(v);
        on_stack[v] = true;
      } else {
        lowlink[v] = std::min(lowlink[v], lowlink[g.neighbor(v, i - 1)]);
      }

      for (; i < g.outdegree(v); ++i) {
        node_t w = g.neighbor(v, i);
        if (!visited[w]) {
          dfs_stack.emplace(v, i + 1);
          dfs_stack.emplace(w, 0);
          break;
        } else if(on_stack[w]) {
          lowlink[v] = std::min(lowlink[v], order[w]);
        }
      }

      if (i == g.outdegree(v) && order[v] == lowlink[v]) {
        node_t w;
        do {
          w = node_stack.top();
          node_stack.pop();
          on_stack[w] = false;
          ids[w] = index;
        } while (v != w);
        ++index;
      }
    }
  }

  return {index, ids};
}

/// [destructive] extract largest SCC
template <typename GraphType>
GraphType& extract_largest_scc(GraphType &g) {
  auto [num_components, ids] = strongly_connected_components(g);
  std::vector<node_t> num_nodes(num_components);
  for (node_t id : ids) {
    num_nodes[id]++;
  }

  std::vector<bool> filter_list(g.num_nodes());
  node_t max_id = std::max_element(num_nodes.begin(), num_nodes.end()) - num_nodes.begin();
  for (node_t v : g.nodes()) {
    filter_list[v] = ids[v] == max_id;
  }

  return g.filter_nodes(filter_list);
}

/// determine if graph is strongly connected
template <typename GraphType>
bool is_strongly_connected(const GraphType &g) {
  return strongly_connected_components(g).first == 1;
}
} // namespace bgl
