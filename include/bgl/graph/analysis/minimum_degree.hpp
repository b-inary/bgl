#pragma once
#include "bgl/graph/basic_graph.hpp"
#include "bgl/util/all.hpp"
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
  min_degree_eliminator() {}

  /// constructor
  min_degree_eliminator(const graph &g, int threshold)
      : min_degree_eliminator(g.clone(), threshold) {}

  /// constructor
  min_degree_eliminator(graph &&g, int threshold)
      : g_{std::move(g)},
        parent_(g.num_nodes()),
        is_hub_(g.num_nodes(), false),
        is_dead_(g.num_nodes(), false) {
    std::iota(parent_.begin(), parent_.end(), static_cast<node_t>(0));
    if (threshold > 0) {
      do_contraction_loop(threshold);
    } else {
      width_ends_.push_back(0);
    }
    complete_ordering();
    clear();
  }

  const std::vector<node_t> &ordering() const { return order_; }
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
      (is_hub_[w] ? vs_hub : vs_normal).push_back(w);
    }

    is_hub_[v] = true;
    g_.mutable_edges(v) = std::move(vs_normal);

    node_t w = v;
    for (node_t u : vs_hub) {
      w = merge_hubs(w, u);
    }

    remove_duplicates(g_.mutable_edges(w));
    remove_elements(g_.mutable_edges(w), v);
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

  void do_contraction_loop(std::size_t threshold) {
    using triple_t = std::tuple<std::size_t, rng_t::result_type, node_t>;
    using queue_t = std::priority_queue<triple_t, std::vector<triple_t>, std::greater<triple_t>>;

    std::vector<triple_t> que_gen;
    for (node_t v : g_.nodes()) {
      que_gen.emplace_back(g_.outdegree(v), bgl_random(), v);
    }
    queue_t que({}, std::move(que_gen));

    std::size_t cur_width = 0;
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

  void complete_ordering() {
    std::vector<bool> ordered(g_.num_nodes());
    for (node_t v : g_.nodes()) {
      if (!is_hub_[v] && !is_dead_[v]) {
        order_.push_back(v);
      }
    }
  }

  void clear() {
    g_.clear();
    parent_.clear();
    is_dead_.clear();
    is_hub_.clear();
  }
};

class min_degree_eliminator_directed {
public:
  min_degree_eliminator_directed() {}

  /// constructor
  min_degree_eliminator_directed(const graph &g, int threshold)
      : min_degree_eliminator_directed(g.clone(), threshold) {}

  /// constructor
  min_degree_eliminator_directed(graph &&g, int threshold)
      : parent_{std::vector<node_t>(g.num_nodes()), std::vector<node_t>(g.num_nodes())},
        is_hub_{std::vector<bool>(g.num_nodes(), false), std::vector<bool>(g.num_nodes(), false)},
        is_dead_{std::vector<bool>(g.num_nodes(), false), std::vector<bool>(g.num_nodes(), false)} {
    g_[0] = std::move(g);
    g_[1] = g_[0].clone().transpose();
    std::iota(parent_[0].begin(), parent_[0].end(), static_cast<node_t>(0));
    std::iota(parent_[1].begin(), parent_[1].end(), static_cast<node_t>(0));
    if (threshold > 0) {
      do_contraction_loop(threshold);
    } else {
      width_ends_.push_back(0);
    }
    complete_ordering();
    clear();
  }

  const std::vector<node_t> &ordering() const { return order_; }
  const std::vector<node_t> &width_ends() const { return width_ends_; }

private:
  graph g_[2];
  std::vector<node_t> parent_[2];
  std::vector<bool> is_hub_[2];
  std::vector<bool> is_dead_[2];
  std::vector<node_t> order_;
  std::vector<node_t> width_ends_;

  node_t find_parent_hub(node_t v, int t) {
    if (parent_[t][v] == v) return v;
    return parent_[t][v] = find_parent_hub(parent_[t][v], t);
  }

  void purify(node_t v, int t) {
    auto &es = g_[t].mutable_edges(v);
    for (node_t &w : es) {
      if (is_dead_[t][w]) w = find_parent_hub(w, t);
    }
    remove_duplicates(es);
  }

  node_t merge_hubs(node_t v, node_t w, int t) {
    if (g_[t].outdegree(v) < g_[t].outdegree(w)) std::swap(v, w);
    auto &vs = g_[t].mutable_edges(v);
    auto &ws = g_[t].mutable_edges(w);
    vs.insert(vs.end(), ws.begin(), ws.end());
    ws.clear();
    ws.shrink_to_fit();
    is_dead_[t][w] = true;
    parent_[t][w] = v;
    return v;
  }

  void contract(node_t v, int t) {
    // |v| is purified (by |get_degree()|)

    std::vector<node_t> vs_normal, vs_hub;
    for (node_t w : g_[t].neighbors(v)) {
      (is_hub_[t][w] ? vs_hub : vs_normal).push_back(w);
    }

    is_hub_[t][v] = true;
    g_[t].mutable_edges(v) = std::move(vs_normal);

    node_t w = v;
    for (node_t u : vs_hub) {
      w = merge_hubs(w, u, t);
    }

    remove_duplicates(g_[t].mutable_edges(w));
    remove_elements(g_[t].mutable_edges(w), v);
  }

  std::size_t get_degree(node_t v, int t) {
    std::vector<node_t> neighbors;
    purify(v, t);

    for (node_t w : g_[t].neighbors(v)) {
      if (is_hub_[t][w]) {
        for (node_t u : g_[t].neighbors(w)) {
          if (u != v) neighbors.push_back(u);
        }
      } else {
        neighbors.push_back(w);
      }
    }

    return remove_duplicates(neighbors).size();
  }

  void do_contraction_loop(std::size_t threshold) {
    using triple_t = std::tuple<std::size_t, rng_t::result_type, node_t>;
    using queue_t = std::priority_queue<triple_t, std::vector<triple_t>, std::greater<triple_t>>;

    std::vector<triple_t> que_gen;
    for (node_t v : g_[0].nodes()) {
      std::size_t outdeg = g_[0].outdegree(v);
      std::size_t indeg = g_[1].outdegree(v);
      que_gen.emplace_back(std::max(outdeg, indeg), bgl_random(), v);
    }
    queue_t que({}, std::move(que_gen));

    std::size_t cur_width = 0;
    while (!que.empty()) {
      node_t v = std::get<2>(que.top());
      que.pop();
      std::size_t outdeg = get_degree(v, 0);
      std::size_t indeg = get_degree(v, 1);
      std::size_t maxdeg = std::max(outdeg, indeg);

      if (!que.empty() && maxdeg > std::get<0>(que.top())) {
        que.emplace(maxdeg, bgl_random(), v);
        continue;
      }

      while (cur_width < maxdeg) {
        width_ends_.push_back(order_.size());
        if (++cur_width > threshold) return;
      }

      contract(v, 0);
      contract(v, 1);
      order_.push_back(v);
    }

    width_ends_.push_back(order_.size());
  }

  void complete_ordering() {
    std::vector<bool> ordered(g_[0].num_nodes());
    for (node_t v : g_[0].nodes()) {
      if (!is_hub_[0][v] && !is_dead_[0][v]) {
        order_.push_back(v);
      }
    }
  }

  void clear() {
    g_[0].clear();
    g_[1].clear();
    parent_[0].clear();
    parent_[1].clear();
    is_dead_[0].clear();
    is_dead_[1].clear();
    is_hub_[0].clear();
    is_hub_[1].clear();
  }
};
}  // namespace bgl
