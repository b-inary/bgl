#pragma once
#include "extlib/radix_heap.hpp"
#include "basic_graph.hpp"
#include <functional>
#include <limits>

namespace bgl {
/// radix heap specialized for Dijkstra's algorithm
/// @see https://github.com/iwiwi/agl/blob/master/src/heap/dijkstra_heap.h
template <typename GraphType>
class dijkstra_heap {
public:
  using graph_type = GraphType;
  using weight_type = typename GraphType::weight_type;

  explicit dijkstra_heap(const graph_type &g)
    : ws_(g.num_nodes(), std::numeric_limits<weight_type>::max()) {}

  bool decrease(node_t v, weight_type w) {
    if (is_le(ws_[v], w)) return false;
    ws_[v] = w;
    h_.push(w, v);
    return true;
  }

  void pop() {
    vs_.emplace_back(top_vertex());
    h_.pop();
  }

  void clear() {
    for (auto v : vs_) ws_[v] = std::numeric_limits<weight_type>::max();
    vs_.clear();
    while (!h_.empty()) {
      ws_[top_vertex()] = std::numeric_limits<weight_type>::max();
      h_.pop();
    }
    h_.clear();
  }

  bool empty() {
    canonicalize();
    return h_.empty();
  }

  node_t top_vertex()  {
    canonicalize();
    return h_.top_value();
  }

  weight_type top_weight() {
    canonicalize();
    return h_.top_key();
  }

private:
  std::vector<node_t> vs_;
  std::vector<weight_type> ws_;
  radix_heap::pair_radix_heap<weight_type, node_t> h_;

  void canonicalize() {
    while (!h_.empty()) {
      if (ws_[h_.top_value()] == h_.top_key()) break;
      h_.pop();
    }
  }
};

// general visitor
template <typename GraphType>
class visitor_by_distance {
public:
  using graph_type = GraphType;
  using weight_type = typename GraphType::weight_type;

  visitor_by_distance(const graph_type &g) : g_(g), h_(g) {}

  void visit(node_t source, std::function<bool(node_t, weight_type)> pred) {
    h_.decrease(source, 0);

    while (!h_.empty()) {
      node_t v = h_.top_vertex();
      weight_type w = h_.top_weight();
      h_.pop();

      if (!pred(v, w)) continue;

      for (const auto &e : g_.edges(v)) {
        h_.decrease(to(e), w + weight(e));
      }
    }

    h_.clear();
  }

 private:
  const graph_type &g_;
  dijkstra_heap<graph_type> h_;
};

// visitor specialized for unweighted graphs
template <>
class visitor_by_distance<graph> {
 public:
  using graph_type = graph;
  using weight_type = graph_type::weight_type;

  visitor_by_distance(const graph_type &g)
    : g_(g)
    , queue_(g.num_nodes())
    , visited_(g.num_nodes(), false) {}

  void visit(node_t source, std::function<bool(node_t, weight_type)> pred) {
    std::size_t head = 0, tail = 0, boundary = 1;
    weight_type w = 0;
    queue_[tail++] = source;
    visited_[source] = true;

    while (head < tail) {
      if (head == boundary) {
        boundary = tail;
        ++w;
      }

      node_t v = queue_[head++];
      if (!pred(v, w)) continue;

      for (node_t u : g_.neighbors(v)) {
        if (visited_[u]) continue;
        queue_[tail++] = u;
        visited_[u] = true;
      }
    }

    // clear
    for (std::size_t i : irange(tail)) {
      visited_[queue_[i]] = false;
    }
  }

private:
  const graph_type &g_;
  std::vector<node_t> queue_;
  std::vector<bool> visited_;   // performance of vector<bool> is not so bad
};

/// visit nodes of graph |g| from node |source| by distance.
/// on each visited node, |pred| is called and stop visiting if its return value is false.
/// time complexity of initialization: O(n)
/// @param g input graph
/// @param source first visited node
/// @param pred predicate function: bool(node_t, weight_type)
template <typename GraphType>
void visit_by_distance(const GraphType &g, node_t source,
                       std::function<bool(node_t, typename GraphType::weight_type)> pred) {
  visitor_by_distance<GraphType>(g).visit(source, pred);
}

/// compute single-source shortest-path distances
template <typename GraphType>
std::vector<typename GraphType::weight_type>
single_source_distance(const GraphType &g, node_t source) {
  using weight_type = typename GraphType::weight_type;
  std::vector<weight_type> result(g.num_nodes(), std::numeric_limits<weight_type>::max());
  visit_by_distance(g, source, lambda(v, w) {
    result[v] = w;
    return true;
  });
  return result;
}
} // namespace bgl
