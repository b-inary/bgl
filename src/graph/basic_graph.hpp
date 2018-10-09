#pragma once
#include "util/all.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <optional>
#include <iterator>
#include <cstdlib>
#include <cstdint>

namespace bgl {

/*
 * Node type
 */

//! type for representing node
using node_t = std::uint32_t;


/*
 * Edge type
 */

//! type for representing unweighted edge
using unweighted_edge_t = node_t;

//! type for representing weighted edge
template <typename WeightType>
using weighted_edge_t = std::pair<node_t, WeightType>;

inline constexpr node_t to(const unweighted_edge_t &e) noexcept {
  return e;
}

template <typename WeightType>
constexpr node_t to(const weighted_edge_t<WeightType> &e) noexcept {
  return e.first;
}

inline constexpr int weight(const unweighted_edge_t &e [[maybe_unused]]) noexcept {
  return 1;
}

template <typename WeightType>
constexpr WeightType weight(const weighted_edge_t<WeightType> &e) noexcept {
  return e.second;
}

/* utility function of edge */
inline void update_to(unweighted_edge_t &e, node_t v) {
  e = v;
}

template <typename WeightType>
inline void update_to(weighted_edge_t<WeightType> &e, node_t v) {
  e.first = v;
}

inline void update_weight(unweighted_edge_t &e [[maybe_unused]], int w [[maybe_unused]]) {
  require_msg(false, "cannot change weight of unweighted edge");
}

template <typename WeightType>
inline void update_weight(weighted_edge_t<WeightType> &e, WeightType w) {
  e.second = w;
}

inline constexpr unweighted_edge_t
reverse_edge(node_t v, unweighted_edge_t e [[maybe_unused]]) noexcept {
  return v;
}

template <typename WeightType>
constexpr weighted_edge_t<WeightType>
reverse_edge(node_t v, weighted_edge_t<WeightType> e) noexcept {
  return {v, weight(e)};
}

template <typename EdgeType>
bool compare_to(EdgeType e, node_t v) {
  return to(e) < v;
}

//! type for representing edge list
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
size_t num_edges(const edge_list<EdgeType> &es) {
  return es.size();
}

//! type for representing adjacency list
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
size_t num_edges(const adjacency_list<EdgeType> &adj) {
  size_t n = 0;
  for (const auto &es : adj) {
    n += es.size();
  }
  return n;
}

//! convert edge list to adjacency list
template <typename EdgeType>
adjacency_list<EdgeType>
convert_to_adjacency_list(node_t num_nodes, const edge_list<EdgeType> &es) {
  adjacency_list<EdgeType> edges(num_nodes);
  for (const auto &e : es) {
    require_msg(0 <= e.first && e.first < num_nodes, "invalid node index");
    require_msg(0 <= to(e.second) && to(e.second) < num_nodes, "invalid node index");
    edges[e.first].push_back(e.second);
  }
  for (auto &es : edges) {
    std::sort(es.begin(), es.end());
  }
  return edges;
}

//! convert adjacency list to edge list
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
  node_t operator*() const noexcept { return to(*static_cast<const Iterator&>(*this)); }
};

//! neighbor adapter
template <typename EdgeType>
class neighbor_adapter {
public:
  neighbor_adapter(const std::vector<EdgeType> &edges) : edges_{edges} {}
  auto begin() const { return neighbor_iterator{std::begin(edges_)}; }
  auto end() const { return neighbor_iterator{std::end(edges_)}; }
private:
  const std::vector<EdgeType> &edges_;
};

/**
 * Basic graph class<br>
 * Internally, graph is stored as sorted adjacency list: vector<vector<edge_type>>.
 * @brief type for representing unweighted/weighted graph
 */
template <typename EdgeType>
class basic_graph {
public:
  using edge_type = EdgeType;
  using weight_type = decltype(weight(edge_type()));
  using graph_type = basic_graph<EdgeType>;

  /* initialization */

  //! default constructor
  basic_graph() : num_nodes_{0}, num_edges_{0} {}

  //! construct with edge list
  basic_graph(const edge_list<edge_type> &es) {
    assign(es);
  }

  //! construct with edge list specifying the number of nodes
  basic_graph(node_t num_nodes, const edge_list<edge_type> &es) {
    assign(num_nodes, es);
  }

  //! construct with adjacency list
  basic_graph(const adjacency_list<edge_type> &adj) {
    assign(adj);
  }

  /**
   * when the number of nodes and edges are not correct, behavior is undefined. |adj| must be rvalue
   * @brief construct with sorted adjacency list specifying the number of nodes and edges.
   */
  basic_graph(node_t num_nodes, std::size_t num_edges, adjacency_list<edge_type> &&adj) {
    assign(num_nodes, num_edges, std::move(adj));
  }

  //! initialize graph with edge list
  void assign(const edge_list<edge_type> &es) {
    assign(bgl::num_nodes(es), es);
  }

  //! initialize graph with edge list specifying the number of nodes
  void assign(node_t num_nodes, const edge_list<edge_type> &es) {
    num_nodes_ = num_nodes;
    num_edges_ = bgl::num_edges(es);
    adj_ = convert_to_adjacency_list(num_nodes, es);
  }

  //! initialize with adjacency list
  void assign(const adjacency_list<edge_type> &adj) {
    num_nodes_ = bgl::num_nodes(adj);
    num_edges_ = bgl::num_edges(adj);
    adj_ = adj;
    for (node_t v : nodes()) {
      auto &es = adj_[v];
      std::sort(es.begin(), es.end());
      require_msg(to(es.back()) < num_nodes_, "invalid index");
    }
  }

  /**
   * when the number of nodes and edges are not correct, behavior is undefined. |adj| must be rvalue
   * @brief initialize with sorted adjacency list specifying the number of nodes and edges.
   */
  void assign(node_t num_nodes, std::size_t num_edges, adjacency_list<edge_type> &&adj) {
    num_nodes_ = num_nodes;
    num_edges_ = num_edges;
    adj_ = std::move(adj);
  }

  //! remove all nodes
  void clear() {
    num_nodes_ = 0;
    num_edges_ = 0;
    adj_.clear();
    adj_.shrink_to_fit();
  }

  //! resize graph
  void resize(node_t new_num_nodes) {
    if (new_num_nodes < num_nodes_) {
      for (node_t v : irange(new_num_nodes)) {
        const auto &es = edges_from(v);
        auto it = lower_bound(es.begin(), es.end(), new_num_nodes, compare_to<edge_type>);
        num_edges_ -= es.end() - it;
        es.erase(it, es.end());
        es.shrink_to_fit();
      }
    }
    for (node_t v : irange(new_num_nodes, num_nodes_)) {
      num_edges_ -= outdegree(v);
    }
    num_nodes_ = new_num_nodes;
    adj_.resize(new_num_nodes);
    adj_.shrink_to_fit();
  }

  /* basic operations */

  //! equality operator
  bool operator==(const graph_type &rhs) const noexcept {
    return num_nodes_ == rhs.num_nodes_
        && num_edges_ == rhs.num_edges_
        && adj_ == rhs.adj_;
  }

  //! inequality operator
  bool operator!=(const graph_type &rhs) const noexcept {
    return !(*this == rhs);
  }

  //! return the number of nodes
  node_t num_nodes() const noexcept {
    return num_nodes_;
  };

  //! return the number of **directed** edges
  std::size_t num_edges() const noexcept {
    return num_edges_;
  };

  //! return outdegree of node |v|
  size_t outdegree(node_t v) const noexcept {
    return adj_[v].size();
  };

  //! useful adapter of nodes for range-based for-loop
  irange_type<node_t> nodes() const noexcept {
    return irange(num_nodes());
  };

  //! return edge list from node |v|
  const std::vector<edge_type> &edges_from(node_t v) const noexcept {
    return adj_[v];
  };

  //! useful adapter of neighbors for range-based for-loop
  neighbor_adapter<edge_type> neighbors(node_t v) const noexcept {
    return neighbor_adapter<edge_type>{adj_[v]};
  };

  //! check if there exists an edge from |u| to |v|
  bool is_adjacent(node_t u, node_t v) const noexcept {
    return get_weight(u, v).has_value();
  }

  //! get (smallest) weight of edge from |u| to |v|. if |u| and |v| are not adjacent, return nullopt
  std::optional<weight_type> get_weight(node_t u, node_t v) const noexcept {
    const auto &es = edges_from(u);
    const auto it = std::lower_bound(es.begin(), es.end(), v, compare_to<edge_type>);
    if (it == es.end() || to(*it) != v) {
      return std::nullopt;
    }
    return weight(*it);
  }

  //! get edge list of the graph
  edge_list<edge_type> get_edge_list() const {
    return convert_to_edge_list(adj_);
  }

  //! [destructive] simplify graph (i.e., remove self edges and multiple edges)
  graph_type &simplify() {
    num_edges_ = 0;
    for (node_t v : nodes()) {
      remove_elements_if(adj_[v], lambda(e) { return to(e) == v; });
      remove_duplicates(adj_[v]); // on weighted graphs, remain edges of different weight
      num_edges_ += adj_[v].size();
    }
    return *this;
  }

  //! [destructive] transpose graph
  graph_type &transpose() {
    adjacency_list<edge_type> adj(num_nodes());
    for (node_t v : nodes()) {
      for (const edge_type &e : edges_from(v)) {
        adj[to(e)].push_back(reverse_edge(v, e));
      }
    }
    adj_ = std::move(adj);
    return *this;
  }

  //! [destructive] make graph undirected
  graph_type &make_undirected() {
    std::vector<size_t> outdegrees(num_nodes());
    for (node_t v : nodes()) {
      outdegrees[v] = outdegree(v);
    }
    for (node_t v : nodes()) {
      for (size_t i : irange(outdegrees[v])) {
        const edge_type &e = adj_[v][i];
        adj_[to(e)].push_back(reverse_edge(v, e));
      }
    }
    num_edges_ = 0;
    for (node_t v : nodes()) {
      auto &es = adj_[v];
      std::sort(es.begin(), es.end());
      remove_duplicates(adj_[v]);
      num_edges_ += adj_[v].size();
    }
    return *this;
  }

  /* pretty print */

  //! do pretty print to |os|
  void pretty_print(std::ostream &os = std::cerr) const {
    static const node_t kLimitNumNodes = 5;
    static const std::size_t kLimitNumEdges = 10;

    fmt::print(os, "====================\n");
    fmt::print(os, "  type: {}\n", typename_of(graph_type{}));
    fmt::print(os, "  # of nodes: {}\n", num_nodes());
    fmt::print(os, "  # of edges: {}\n", num_edges());
    fmt::print(os, "--------------------\n");
    for (node_t v : irange(std::min(num_nodes(), kLimitNumNodes))) {
      fmt::print(os, "  {} -> ", v);
      for (size_t i : irange(std::min(outdegree(v), kLimitNumEdges))) {
        if (i > 0) fmt::print(os, ", ");
        fmt::print(os, "{}", edges_from(v)[i]);
      }
      if (outdegree(v) > kLimitNumEdges) fmt::print(os, ", ...");
      fmt::print(os, "\n");
    }
    if (num_nodes() > kLimitNumNodes) fmt::print(os, "  ...\n");
    fmt::print(os, "====================\n");
  }

private:
  adjacency_list<edge_type> adj_;
  node_t num_nodes_;
  std::size_t num_edges_;
};

//! specialized type for representing unweighted graph
using graph = basic_graph<unweighted_edge_t>;

//! specialized type for representing weighted graph
template <typename WeightType>
using wgraph = basic_graph<weighted_edge_t<WeightType>>;

} // namespace bgl


// istream/ostream support of weighted_edge_t
namespace std {
template <typename WeightType>
istream &operator<<(istream &is, bgl::weighted_edge_t<WeightType> &e) {
  return is >> e.first >> e.second;
}

template <typename WeightType>
ostream &operator<<(ostream &os, const bgl::weighted_edge_t<WeightType> &e) {
  return os << bgl::to(e) << ' ' << bgl::weight(e);
}
} // namespace std
