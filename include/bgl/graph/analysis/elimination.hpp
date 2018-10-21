#pragma once
#include "bgl/util/all.hpp"
#include "bgl/graph/basic_graph.hpp"
#include <queue>
#include <tuple>

namespace bgl {
/*
 *  @see "Computing personalized PageRank quickly by exploiting graph structures"
 *       (T. Maehara et al.). In VLDB'14.
 *
 *  There are three types of nodes: normal, hub, dead.
 *  When a normal node is eliminated, it becomes hub.
 *  When two hub nodes are merged, one of the hub node becomes dead.
 *
 *  [possible connection]
 *    - normal -> normal, hub, dead
 *    - hub -> normal
 *    - dead -> hub, dead (parent link)
 *
 *  |purify()| deletes normal -> dead links.
 */
class min_degree_eliminator {
public:
  /// constructor: input graph must be undirected
  min_degree_eliminator(const graph &g, std::size_t threshold)
    : min_degree_eliminator(g.clone(), threshold) {}

  min_degree_eliminator(graph &&g, std::size_t threshold)
    : g_{std::move(g)}
    , parent_(g.num_nodes())
    , is_hub_(g.num_nodes(), false)
    , is_dead_(g.num_nodes(), false)
  {
    std::iota(parent_.begin(), parent_.end(), static_cast<node_t>(0));
    preprocess();
    do_contraction_loop(threshold);
  }

  const std::vector<node_t> &partial_ordering() const { return order_; }
  const std::vector<node_t> &width_ends() const { return width_ends_; }

private:
  graph g_;
  std::vector<node_t> parent_;
  std::vector<bool> is_hub_;
  std::vector<bool> is_dead_;
  std::vector<node_t> order_;
  std::vector<node_t> width_ends_;

  node_t find_parent_hub(node_t v) {
    if (parent_[v] == v) return v;
    return parent_[v] = find_parent_hub(parent_[v]);
  }

  void purify(node_t v) {
    auto &es = g_.mutable_edges(v);
    for (node_t &w : es) {
      if (is_dead_[w]) w = find_parent_hub(w);
    }
    remove_duplicates(es);
  }

  node_t merge_hubs(node_t v, node_t w) {
    if (g_.outdegree(v) < g_.outdegree(w)) std::swap(v, w);
    auto &vs = g_.mutable_edges(v);
    auto &ws = g_.mutable_edges(w);
    vs.insert(vs.end(), ws.begin(), ws.end());
    ws.clear();
    ws.shrink_to_fit();
    is_dead_[w] = true;
    parent_[w] = v;
    return v;
  }

  void contract(node_t v) {
    // |v| is purified (by |get_degree()|)

    std::vector<node_t> vs_normal, vs_hub;
    for (node_t w : g_.neighbors(v)) {
      if (is_hub_[w]) {
        vs_hub.push_back(w);
        remove_elements(g_.mutable_edges(w), v);
      } else {
        vs_normal.push_back(w);
      }
    }

    is_hub_[v] = true;
    g_.mutable_edges(v) = std::move(vs_normal);
    for (node_t w : vs_hub) {
      v = merge_hubs(v, w);
    }

    remove_duplicates(g_.mutable_edges(v));
  }

  std::size_t get_degree(node_t v) {
    std::vector<node_t> neighbors;
    purify(v);

    for (node_t w : g_.neighbors(v)) {
      if (is_hub_[w]) {
        for (node_t u : g_.neighbors(w)) {
          if (u != v) neighbors.push_back(u);
        }
      } else {
        neighbors.push_back(w);
      }
    }

    return remove_duplicates(neighbors).size();
  }

  void preprocess() {
    std::vector<bool> alive(g_.num_nodes(), true);

    // remove zero-degree vertices
    for (node_t v : g_.nodes()) {
      if (g_.outdegree(v) == 0) {
        alive[v] = false;
        order_.push_back(v);
      }
    }
    width_ends_.push_back(order_.size());

    // remove one-degree nodes
    size_t num_ordered;
    std::vector<node_t> minus_degree(g_.num_nodes(), 0);
    do {
      num_ordered = order_.size();
      for (node_t v : g_.nodes()) {
        if (alive[v] && g_.outdegree(v) - minus_degree[v] <= 1) {
          for (node_t w : g_.neighbors(v)) {
            ++minus_degree[w];
          }
          alive[v] = false;
          order_.push_back(v);
        }
      }
    } while (num_ordered < order_.size());
    width_ends_.push_back(order_.size());

    // construct preprocessed graph
    for (node_t v : g_.nodes()) {
      auto &es = g_.mutable_edges(v);
      if (alive[v]) filter(es, lambda(w) { return alive[w]; });
      else es.clear();
    }
  }

  void do_contraction_loop(std::size_t threshold) {
    using triple_t = std::tuple<std::size_t, rng_t::result_type, node_t>;
    using queue_t = std::priority_queue<triple_t, std::vector<triple_t>, std::greater<triple_t>>;

    std::vector<triple_t> que_gen;
    for (node_t v : g_.nodes()) {
      std::size_t deg = g_.outdegree(v);
      if (deg && deg < threshold) que_gen.emplace_back(deg, bgl_random(), v);
    }
    queue_t que({}, std::move(que_gen));

    std::size_t cur_width = 2;
    while (!que.empty()) {
      node_t v = std::get<2>(que.top());
      que.pop();
      std::size_t deg = get_degree(v);

      if (!que.empty() && deg > std::get<0>(que.top())) {
        que.emplace(deg, bgl_random(), v);
        continue;
      }

      while (cur_width < deg) {
        width_ends_.push_back(order_.size());
        if (++cur_width > threshold) return;
      }

      contract(v);
      order_.push_back(v);
    }

    width_ends_.push_back(order_.size());
  }
};
} // namespace bgl
