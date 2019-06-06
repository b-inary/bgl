#pragma once
#include "bgl/util/all.hpp"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace bgl {

/*
 * Node type
 */

/// type for representing node
using node_t = std::uint32_t;


/*
 * Edge type
 */

/// type for representing unweighted edge
using unweighted_edge_t = node_t;

/// type for representing weighted edge
template <typename WeightType>
using weighted_edge_t = std::pair<node_t, WeightType>;

inline constexpr node_t to(const unweighted_edge_t &e) noexcept { return e; }

template <typename WeightType>
constexpr node_t to(const weighted_edge_t<WeightType> &e) noexcept {
  return e.first;
}

inline constexpr int weight(const unweighted_edge_t &) noexcept { return 1; }

template <typename WeightType>
constexpr WeightType weight(const weighted_edge_t<WeightType> &e) noexcept {
  return e.second;
}

/* utility functions of edge */
inline constexpr unweighted_edge_t update_to(const unweighted_edge_t &, node_t v) noexcept {
  return v;
}

template <typename WeightType>
constexpr weighted_edge_t<WeightType> update_to(const weighted_edge_t<WeightType> &e,
                                                node_t v) noexcept {
  return {v, weight(e)};
}

[[noreturn]] inline void update_weight(const unweighted_edge_t &) noexcept {
  ASSERT_MSG(false, "called with unweighted edge");
}

template <typename WeightType>
constexpr weighted_edge_t<WeightType> update_weight(const weighted_edge_t<WeightType> &e,
                                                    const WeightType &w) noexcept {
  return {to(e), w};
}

template <typename EdgeType>
bool compare_edge_node(const EdgeType &e, node_t v) {
  return to(e) < v;
}

template <typename EdgeType>
bool compare_node_edge(node_t v, const EdgeType &e) {
  return v < to(e);
}

/// type for representing edge list
template <typename EdgeType>
using edge_list = std::vector<std::pair<node_t, EdgeType>>;

using unweighted_edge_list = edge_list<unweighted_edge_t>;

template <typename WeightType>
using weighted_edge_list = edge_list<weighted_edge_t<WeightType>>;

template <typename EdgeType>
node_t num_nodes(const edge_list<EdgeType> &es) {
  node_t n = 0;
  for (const auto &e : es) {
    n = std::max(n, std::max(e.first, to(e.second)) + 1);
  }
  return n;
}

template <typename EdgeType>
std::size_t num_edges(const edge_list<EdgeType> &es) {
  return es.size();
}

/// type for representing adjacency list
template <typename EdgeType>
using adjacency_list = std::vector<std::vector<EdgeType>>;

using unweighted_adjacency_list = adjacency_list<unweighted_edge_t>;

template <typename WeightType>
using weighted_adjacency_list = adjacency_list<weighted_edge_t<WeightType>>;

template <typename EdgeType>
node_t num_nodes(const adjacency_list<EdgeType> &adj) {
  return adj.size();
}

template <typename EdgeType>
std::size_t num_edges(const adjacency_list<EdgeType> &adj) {
  std::size_t n = 0;
  for (const auto &es : adj) {
    n += es.size();
  }
  return n;
}

/// convert edge list to adjacency list
template <typename EdgeType>
adjacency_list<EdgeType> convert_to_adjacency_list(node_t num_nodes,
                                                   const edge_list<EdgeType> &es) {
  adjacency_list<EdgeType> edges(num_nodes);
  for (const auto &e : es) {
    ASSERT_MSG(0 <= e.first && e.first < num_nodes, "invalid node index");
    ASSERT_MSG(0 <= to(e.second) && to(e.second) < num_nodes, "invalid node index");
    edges[e.first].push_back(e.second);
  }
  for (auto &es : edges) {
    std::sort(es.begin(), es.end());
  }
  return edges;
}

/// convert adjacency list to edge list
template <typename EdgeType>
edge_list<EdgeType> convert_to_edge_list(const adjacency_list<EdgeType> &adj) {
  node_t n = num_nodes(adj);
  edge_list<EdgeType> es;
  for (node_t v : irange(n)) {
    for (const auto &e : adj[v]) {
      es.emplace_back(v, e);
    }
  }
  return es;
}


/*
 * Graph type
 */

template <typename Iterator>
class neighbor_iterator : public Iterator {
public:
  neighbor_iterator(Iterator it) : Iterator{it} {}
  node_t operator*() const noexcept { return to(*static_cast<const Iterator &>(*this)); }
};

/// neighbor adapter
template <typename EdgeType>
class neighbor_adapter {
public:
  neighbor_adapter(const std::vector<EdgeType> &edges) : edges_{edges} {}
  auto begin() const { return neighbor_iterator{std::begin(edges_)}; }
  auto end() const { return neighbor_iterator{std::end(edges_)}; }

private:
  const std::vector<EdgeType> &edges_;
};

/// Basic graph class: type for representing unweighted/weighted graph.
/// Internally, graph is stored as sorted adjacency list: vector<vector<edge_type>>.
template <typename EdgeType>
class basic_graph {
public:
  using edge_type = EdgeType;
  using weight_type = decltype(weight(edge_type{}));
  using graph_type = basic_graph<EdgeType>;

  /* initialization */

  /// default constructor
  basic_graph() : num_nodes_{0}, num_edges_{0} {}

  /// construct with edge list
  basic_graph(const edge_list<edge_type> &es) { assign(es); }

  /// construct with edge list specifying the number of nodes
  basic_graph(node_t num_nodes, const edge_list<edge_type> &es = {}) { assign(num_nodes, es); }

  /// construct with adjacency list
  basic_graph(const adjacency_list<edge_type> &adj) { assign(adj); }

  /// construct with adjacency list
  basic_graph(adjacency_list<edge_type> &&adj) { assign(std::move(adj)); }

  /// construct with sorted adjacency list specifying the number of nodes and edges.
  /// when the number of nodes and edges are not correct, behavior is undefined.
  /// |adj| must be rvalue
  basic_graph(node_t num_nodes, std::size_t num_edges, adjacency_list<edge_type> &&adj) {
    assign(num_nodes, num_edges, std::move(adj));
  }

  /// initialize graph with edge list
  graph_type &assign(const edge_list<edge_type> &es) { return assign(bgl::num_nodes(es), es); }

  /// initialize graph with edge list specifying the number of nodes
  graph_type &assign(node_t num_nodes, const edge_list<edge_type> &es) {
    num_nodes_ = num_nodes;
    num_edges_ = bgl::num_edges(es);
    adj_ = convert_to_adjacency_list(num_nodes, es);
    return *this;
  }

  /// initialize with adjacency list
  graph_type &assign(const adjacency_list<edge_type> &adj) {
    auto adj_copy = adj;
    return assign(std::move(adj_copy));
  }

  /// initialize with adjacency list
  graph_type &assign(adjacency_list<edge_type> &&adj) {
    num_nodes_ = bgl::num_nodes(adj);
    num_edges_ = bgl::num_edges(adj);
    adj_ = std::move(adj);
    for (node_t v : nodes()) {
      auto &es = mutable_edges(v);
      if (es.size() > 0) {
        std::sort(es.begin(), es.end());
        ASSERT_MSG(to(es.back()) < num_nodes_, "invalid index");
      }
    }
    return *this;
  }

  /// initialize with sorted adjacency list specifying the number of nodes and edges.
  /// when the number of nodes and edges are not correct, behavior is undefined.
  /// |adj| must be rvalue
  graph_type &assign(node_t num_nodes, std::size_t num_edges, adjacency_list<edge_type> &&adj) {
    num_nodes_ = num_nodes;
    num_edges_ = num_edges;
    adj_ = std::move(adj);
    return *this;
  }

  /// remove all nodes
  graph_type &clear() {
    num_nodes_ = 0;
    num_edges_ = 0;
    adj_.clear();
    adj_.shrink_to_fit();
    return *this;
  }

  /// resize graph
  graph_type &resize(node_t new_num) {
    if (new_num < num_nodes()) {
      num_edges_ = 0;
      for (node_t v : irange(new_num)) {
        auto &es = mutable_edges(v);
        auto it = std::lower_bound(es.begin(), es.end(), new_num, compare_edge_node<edge_type>);
        num_edges_ += it - es.begin();
        es.erase(it, es.end());
      }
    }
    num_nodes_ = new_num;
    adj_.resize(new_num);
    adj_.shrink_to_fit();
    return *this;
  }

  /// make clone
  graph_type clone() const { return *this; }

  /* basic operations */

  /// equality operator
  bool operator==(const graph_type &rhs) const noexcept {
    return num_nodes_ == rhs.num_nodes_ && num_edges_ == rhs.num_edges_ && adj_ == rhs.adj_;
  }

  /// inequality operator
  bool operator!=(const graph_type &rhs) const noexcept { return !(*this == rhs); }

  /// return the number of nodes
  node_t num_nodes() const noexcept { return num_nodes_; };

  /// return the number of **directed** edges
  std::size_t num_edges() const noexcept { return num_edges_; };

  /// determine whether graph is empty
  bool empty() const noexcept { return num_nodes() == 0; }

  /// return outdegree of node |v|
  std::size_t outdegree(node_t v) const noexcept { return adj_[v].size(); };

  /// useful adapter of nodes for range-based for-loop
  irange_type<node_t> nodes() const noexcept { return irange(num_nodes()); };

  /// return an edge from node |v| of index |i|
  const edge_type &edge(node_t v, std::size_t i) const noexcept { return adj_[v][i]; };

  /// return edge list from node |v|
  const std::vector<edge_type> &edges(node_t v) const noexcept { return adj_[v]; };

  /// return **mutable** edge list from node |v| (use carefully!)
  std::vector<edge_type> &mutable_edges(node_t v) noexcept { return adj_[v]; }

  /// return a neighbor from node |v| of index |i|
  node_t neighbor(node_t v, std::size_t i) const noexcept { return to(adj_[v][i]); }

  /// useful adapter of neighbors for range-based for-loop
  neighbor_adapter<edge_type> neighbors(node_t v) const noexcept {
    return neighbor_adapter<edge_type>{adj_[v]};
  };

  /// check if there exists an edge from |u| to |v|
  bool is_adjacent(node_t u, node_t v) const noexcept { return get_weight(u, v).has_value(); }

  /// get (smallest) weight of edge from |u| to |v|.
  /// if |u| and |v| are not adjacent, return nullopt
  std::optional<weight_type> get_weight(node_t u, node_t v) const noexcept {
    const auto &es = edges(u);
    const auto it = std::lower_bound(es.begin(), es.end(), v, compare_edge_node<edge_type>);
    if (it == es.end() || to(*it) != v) {
      return std::nullopt;
    }
    return weight(*it);
  }

  /// get edge list of the graph
  edge_list<edge_type> get_edge_list() const { return convert_to_edge_list(adj_); }

  /* parallel for-each loop */

  /// parallel for-each loop
  /// @param callback callback function (must be thread safe): argument is node ID
  /// @param num_threads the number of threads: when specified 0, set automatically
  void for_each_node(std::function<void(node_t)> callback, int num_threads = 0) const {
    std::atomic<int> counter = 0;
    std::vector<std::thread> workers;
    if (num_threads == 0) {
      num_threads = std::min(std::thread::hardware_concurrency(),
                             (num_nodes() + kParallelUnit - 1) / kParallelUnit);
    }

    for (int i[[maybe_unused]] : irange(num_threads)) {
      workers.emplace_back(fn() {
        while (true) {
          int index = counter++;
          node_t start = index * kParallelUnit;
          node_t end = std::min((index + 1) * kParallelUnit, num_nodes());
          for (node_t v : irange(start, end)) {
            callback(v);
          }
          if (end == num_nodes()) return;
        }
      });
    }

    for (int i : irange(num_threads)) {
      workers[i].join();
    }
  }

  /* graph conversion */

  /// [destructive] simplify graph (i.e., remove self loops and multiple edges)
  graph_type &simplify(bool preserve_all_weight = false, bool preserve_self_loops = false) {
    num_edges_ = 0;
    for (node_t v : nodes()) {
      auto &es = mutable_edges(v);
      if (!preserve_self_loops) {
        remove_elements_if(es, fn(e) { return to(e) == v; });
      }
      if (preserve_all_weight) {
        remove_duplicates(es);
      } else {
        remove_duplicates(es, fn(lhs, rhs) { return to(lhs) == to(rhs); });
      }
      es.shrink_to_fit();
      num_edges_ += es.size();
    }
    return *this;
  }

  /// [destructive] transpose graph
  graph_type &transpose() {
    adjacency_list<edge_type> adj(num_nodes());
    for (node_t v : nodes()) {
      auto &es = mutable_edges(v);
      for (const edge_type &e : es) {
        adj[to(e)].push_back(update_to(e, v));
      }
      es.clear();
      es.shrink_to_fit();
    }
    adj_ = std::move(adj);
    return *this;
  }

  /// [destructive] make graph undirected
  graph_type &make_undirected() {
    std::vector<std::size_t> outdegrees(num_nodes());
    for (node_t v : nodes()) {
      outdegrees[v] = outdegree(v);
    }
    for (node_t v : nodes()) {
      for (std::size_t i : irange(outdegrees[v])) {
        const edge_type &e = edge(v, i);
        adj_[to(e)].push_back(update_to(e, v));
      }
    }
    num_edges_ = 0;
    for (node_t v : nodes()) {
      auto &es = mutable_edges(v);
      remove_duplicates(es);
      es.shrink_to_fit();
      num_edges_ += es.size();
    }
    return *this;
  }

  /// [destructive] filter nodes by |filter_list|. can rename ID of nodes
  graph_type &filter_nodes(const std::vector<bool> &filter_list) {
    ASSERT_MSG(num_nodes() == filter_list.size(), "invalid argument");
    std::vector<node_t> perm;
    for (node_t v : nodes()) {
      if (filter_list[v]) perm.push_back(v);
    }
    node_t new_num = perm.size();
    for (node_t v : nodes()) {
      if (!filter_list[v]) perm.push_back(v);
    }
    permute_nodes(perm);
    resize(new_num);
    return *this;
  }

  /// [destructive] remove isolated nodes (i.e., nodes with degree zero). can rename ID of nodes
  graph_type &remove_isolated_nodes() {
    std::vector<bool> is_not_isolated(num_nodes(), false);
    for (node_t u : nodes()) {
      for (node_t v : neighbors(u)) {
        is_not_isolated[u] = true;
        is_not_isolated[v] = true;
      }
    }
    return filter_nodes(is_not_isolated);
  }

  /// [desructive] permute ID of nodes
  /// @param perm node permutation
  graph_type &permute_nodes(const std::vector<node_t> &perm) {
    ASSERT_MSG(num_nodes() == perm.size(), "invalid argument: size does not match");
    std::vector<node_t> cur_perm(num_nodes());
    std::vector<node_t> rev_perm(num_nodes());
    std::vector<bool> dup_check(num_nodes());
    std::iota(cur_perm.begin(), cur_perm.end(), 0);
    std::iota(rev_perm.begin(), rev_perm.end(), 0);

    for (node_t v : nodes()) {
      ASSERT_MSG(0 <= perm[v] && perm[v] < num_nodes(), "out of range: perm[{}] = {}", v, perm[v]);
      ASSERT_MSG(!dup_check[perm[v]], "invalid argument: {} appear twice in perm", perm[v]);
      dup_check[perm[v]] = true;
      node_t tmp = cur_perm[v];
      std::swap(adj_[v], adj_[rev_perm[perm[v]]]);
      std::swap(cur_perm[v], cur_perm[rev_perm[perm[v]]]);
      std::swap(rev_perm[perm[v]], rev_perm[tmp]);
    }

    for (node_t v : nodes()) {
      auto &es = mutable_edges(v);
      for (auto &e : es) {
        e = update_to(e, rev_perm[to(e)]);
      }
      std::sort(es.begin(), es.end());
    }

    return *this;
  }

  /* dynamic update: since adjacency list is sorted, these operations are slow! */

  /// add edge |e| from |v|
  graph_type &add_edge(node_t v, const edge_type &e) {
    auto &es = mutable_edges(v);
    es.insert(std::upper_bound(es.begin(), es.end(), e), e);
    ++num_edges_;
    return *this;
  }

  /// remove all edges from |u| to |v|
  graph_type &remove_edge(node_t u, node_t v) {
    auto &es = mutable_edges(u);
    auto lb = std::lower_bound(es.begin(), es.end(), v, compare_edge_node<edge_type>);
    auto ub = std::upper_bound(es.begin(), es.end(), v, compare_node_edge<edge_type>);
    num_edges_ -= ub - lb;
    es.erase(lb, ub);
    return *this;
  }

  /* pretty print */

  /// do pretty print to |os|
  void pretty_print(std::ostream &os = std::cerr) const {
    const node_t kLimitNumNodes = 5;
    const std::size_t kLimitNumEdges = 10;

    fmt::print(os, "====================\n");
    fmt::print(os, "  # of nodes: {}\n", commify(num_nodes()));
    fmt::print(os, "  # of edges: {}\n", commify(num_edges()));
    fmt::print(os, "  weight type: {}\n", weight_string());
    fmt::print(os, "--------------------\n");
    for (node_t v : irange(std::min(num_nodes(), kLimitNumNodes))) {
      fmt::print(os, "  {} -> ", v);
      for (std::size_t i : irange(std::min(outdegree(v), kLimitNumEdges))) {
        if (i > 0) fmt::print(os, ", ");
        fmt::print(os, "{}", edges(v)[i]);
      }
      if (outdegree(v) > kLimitNumEdges) fmt::print(os, ", ...");
      fmt::print(os, "\n");
    }
    if (num_nodes() > kLimitNumNodes) fmt::print(os, "  ...\n");
    fmt::print(os, "====================\n");
  }

  /* weight information */

  std::string weight_string() const {
    if (std::is_same_v<edge_type, unweighted_edge_t>) return "unweighted";
    return typename_of(weight_type{});
  }

  std::size_t weight_sizeof() const {
    if (std::is_same_v<edge_type, unweighted_edge_t>) return 0;
    return sizeof(weight_type);
  }

private:
  adjacency_list<edge_type> adj_;
  node_t num_nodes_;
  std::size_t num_edges_;
  static const node_t kParallelUnit = 1024;
};

/// specialized type for representing unweighted graph
using graph = basic_graph<unweighted_edge_t>;

/// specialized type for representing weighted graph
template <typename WeightType>
using wgraph = basic_graph<weighted_edge_t<WeightType>>;

/// convert from unweighted graph to weighted graph
template <typename WeightType>
wgraph<WeightType> convert_to_weighted(const graph &g,
                                       std::function<WeightType(node_t, node_t)> weight_fn =
                                           [](node_t, node_t) { return 1; }) {
  weighted_adjacency_list<WeightType> adj(g.num_nodes());
  for (node_t u : g.nodes()) {
    for (node_t v : g.neighbors(u)) {
      adj[u].emplace_back(v, weight_fn(u, v));
    }
  }
  return {g.num_nodes(), g.num_edges(), std::move(adj)};
}

/// convert from weighted graph to unweighted graph (discard weights)
template <typename WeightType>
graph convert_to_unweighted(const wgraph<WeightType> &g) {
  std::size_t num_zero_weighted = 0;
  unweighted_adjacency_list adj(g.num_nodes());
  for (node_t u : g.nodes()) {
    for (auto [v, w] : g.edges(u)) {
      if (is_zero(w)) {
        ++num_zero_weighted;
      } else {
        adj[u].push_back(v);
      }
    }
  }
  return {g.num_nodes(), g.num_edges() - num_zero_weighted, std::move(adj)};
}
}  // namespace bgl
