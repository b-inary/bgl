#pragma once
#include "util/all.hpp"
#include "basic_graph.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <type_traits>
#include <optional>
#include <cstdint>

// istream support of weighted_edge_t
namespace std {
template <typename WeightType>
istream &operator>>(istream &is, bgl::weighted_edge_t<WeightType> &e) {
  return is >> e.first >> e.second;
}
} // namespace std

namespace bgl {

/* tsv I/O */

inline void write_edge_tsv(std::ostream &os, const unweighted_edge_t &e) {
  fmt::print(os, "{}", to(e));
}

template <typename WeightType>
void write_edge_tsv(std::ostream &os, const weighted_edge_t<WeightType> &e) {
  fmt::print(os, "{} {}", to(e), weight(e));
}

/// read graph from |is| as tsv file
/// when type does not match and |accept_mismatch|, then return |nullopt|
template <typename GraphType>
std::optional<GraphType>
read_graph_tsv_optional(std::istream &is, bool rename_id = false, bool accept_mismatch = false) {
  using edge_t = typename GraphType::edge_type;
  ASSERT_MSG(is, "read_graph_tsv: empty stream");

  std::string line;
  edge_list<edge_t> es;
  node_t num_v = 0;
  std::unordered_map<std::size_t, node_t> id;
  bool type_checked = false;

  for (std::size_t lineno = 1; std::getline(is, line); ++lineno) {
    std::size_t v;
    edge_t e;

    // weight type check
    static const std::string type_comment = "# weight type: ";
    if (line.compare(0, type_comment.size(), type_comment) == 0) {
      std::string type_string = line.substr(type_comment.size());
      std::string read_as = GraphType{}.weight_string();
      if (accept_mismatch && type_string != read_as) {
        return std::nullopt;
      }
      ASSERT_MSG(
        type_string == read_as,
        "read_graph_tsv: type of edge weight does not match\n"
        "  read as: {}\n  input type: {}",
        read_as, type_string
      );
      type_checked = true;
    }

    if (line.empty() || line[0] == '#') continue;

    std::stringstream ss(line);
    bool read_success = static_cast<bool>(ss >> v >> e);
    if (!read_success && accept_mismatch && !type_checked) {
      return std::nullopt;
    }

    ASSERT_MSG(
      read_success,
      "read_graph_tsv: read failed at line {}\n  read: {}\n  weight type: {}",
      lineno, line, GraphType{}.weight_string()
    );

    if (rename_id) {
      if (id.count(v) == 0) id[v] = num_v++;
      if (id.count(to(e)) == 0) id[to(e)] = num_v++;
      v = id[v];
      update_to(e, id[to(e)]);
    }

    es.emplace_back(static_cast<node_t>(v), e);
  }

  return GraphType(es);
}

/// read graph from |is| as tsv file
template <typename GraphType>
GraphType read_graph_tsv(std::istream &is, bool rename_id = false) {
  return read_graph_tsv_optional<GraphType>(is, rename_id).value();
}

/// read graph from |filename| as tsv file
/// when type does not match and |accept_mismatch|, then return |nullopt|
template <typename GraphType>
std::optional<GraphType>
read_graph_tsv_optional(path file, bool rename_id = false, bool accept_mismatch = false) {
  std::ifstream ifs(file.string());
  ASSERT_MSG(ifs, "read_graph_tsv: file does not exist: {}", file);
  return read_graph_tsv_optional<GraphType>(ifs, rename_id, accept_mismatch);
}

/// read graph from |filename| as tsv file
template <typename GraphType>
GraphType read_graph_tsv(path filename, bool rename_id = false) {
  return read_graph_tsv_optional<GraphType>(filename, rename_id).value();
}

/// write graph to |os| as tsv file
template <typename GraphType>
void write_graph_tsv(std::ostream &os, const GraphType &g, bool write_info = true) {
  ASSERT_MSG(os, "write_graph_tsv: empty stream");
  if (write_info) {
    fmt::print(os, "# number of nodes: {}\n", g.num_nodes());
    fmt::print(os, "# number of edges: {}\n", g.num_edges());
    fmt::print(os, "# weight type: {}\n", g.weight_string());
  }
  for (node_t v : g.nodes()) {
    for (const auto &e : g.edges_from(v)) {
      fmt::print(os, "{} ", v);
      write_edge_tsv(os, e);
      fmt::print(os, "\n");
    }
  }
  os.flush();
}

/// write graph to |filename| as tsv file
template <typename GraphType>
void write_graph_tsv(path filename, const GraphType &g, bool write_info = true) {
  std::ofstream ofs(filename.string());
  ASSERT_MSG(ofs, "write_graph_tsv: file cannot open: {}", filename);
  write_graph_tsv(ofs, g, write_info);
}


/* binary I/O */

template <typename T>
T read_binary(std::istream &is) {
  T t{};
  is.read(reinterpret_cast<char*>(&t), sizeof(T));
  return t;
}

template <typename T>
void write_binary(std::ostream &os, const T &t) {
  os.write(reinterpret_cast<const char*>(&t), sizeof(T));
}

/// read graph from |is| in binary format .
/// when type does not match and |accept_mismatch|, then return |nullopt|
template <typename GraphType>
std::optional<GraphType>
read_graph_binary_optional(std::istream &is, bool accept_mismatch = false) {
  using edge_t = typename GraphType::edge_type;
  using weight_t = decltype(weight(edge_t{}));
  static_assert(std::is_arithmetic_v<weight_t>, "edge weight must be arithmetic type");
  ASSERT_MSG(is, "read_graph_binary: empty stream");

  // check header
  char buf[4];
  is.read(buf, 4);
  ASSERT_MSG(
    is.gcount() == 4 && buf[0] == 'b' && buf[1] == 'g' && buf[2] == 'l' && buf[3] == '\0',
    "read_graph_binary: invalid header"
  );

  // check weight type
  std::uint32_t edge_size = read_binary<std::uint32_t>(is);
  bool is_integral = read_binary<std::uint32_t>(is);
  bool type_matched = edge_size == GraphType{}.weight_sizeof() &&
                      is_integral == std::is_integral_v<weight_t>;

  if (accept_mismatch && !type_matched) {
    return std::nullopt;
  }

  ASSERT_MSG(
    type_matched,
    "read_graph_binary: type of edge weight does not match\n"
    "  read as: {}\n  input type: size = {} byte(s), is_integral = {}",
    GraphType{}.weight_string(), edge_size, is_integral
  );

  // graph body
  node_t num_nodes = read_binary<node_t>(is);
  std::uint64_t num_edges = read_binary<std::uint64_t>(is);
  adjacency_list<edge_t> adj(num_nodes);
  for (node_t v : irange(num_nodes)) {
    std::uint64_t degree = read_binary<std::uint64_t>(is);
    adj[v].resize(degree);
    is.read(reinterpret_cast<char*>(adj[v].data()), degree * sizeof(edge_t));
  }

  return GraphType(num_nodes, num_edges, std::move(adj));
}

/// read graph from |is| in binary format
template <typename GraphType>
GraphType read_graph_binary(std::istream &is) {
  return read_graph_binary_optional<GraphType>(is).value();
}

/// read graph from |filename| in binary format.
/// when type does not match and |accept_mismatch|, then return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_binary_optional(path filename, bool accept_mismatch = false) {
  std::ifstream ifs(filename.string(), std::ios_base::binary);
  ASSERT_MSG(ifs, "read_graph_binary: file does not exist: {}", filename);
  return read_graph_binary_optional<GraphType>(ifs, accept_mismatch);
}

/// read graph from |filename| in binary format
template <typename GraphType>
GraphType read_graph_binary(path filename) {
  return read_graph_binary_optional<GraphType>(filename).value();
}

/// write graph to |os| in binary format
template <typename GraphType>
void write_graph_binary(std::ostream &os, const GraphType &g) {
  using edge_t = typename GraphType::edge_type;
  using weight_t = decltype(weight(edge_t{}));
  static_assert(std::is_arithmetic_v<weight_t>, "edge weight must be arithmetic type");
  ASSERT_MSG(os, "write_graph_binary: empty stream");

  // header
  os.write("bgl", 4);
  write_binary(os, static_cast<std::uint32_t>(g.weight_sizeof()));
  write_binary(os, static_cast<std::uint32_t>(std::is_integral_v<weight_t>));

  // graph body
  write_binary(os, g.num_nodes());
  write_binary(os, static_cast<std::uint64_t>(g.num_edges()));
  for (node_t v : g.nodes()) {
    write_binary(os, static_cast<std::uint64_t>(g.outdegree(v)));
    os.write(reinterpret_cast<const char*>(g.edges_from(v).data()),
             g.outdegree(v) * sizeof(edge_t));
  }

  os.flush();
}

/// write graph to |filename| in binary format
template <typename GraphType>
void write_graph_binary(path filename, const GraphType &g) {
  std::ofstream ofs(filename.string(), std::ios_base::binary);
  ASSERT_MSG(ofs, "write_graph_binary: file cannot open: {}", filename);
  write_graph_binary(ofs, g);
}


/* traverse directory */

/// read all graphs whose types are |GraphType|
template <typename GraphType>
class graph_folder_iterator {
public:
  graph_folder_iterator() : index_{0} {}
  graph_folder_iterator(path dirname, bool recursive = false, bool rename_id = false)
    : index_{0}, rename_id_{rename_id}
  {
    if (recursive) {
      paths_ = path::find_recursive(dirname, "*.(bgl|tsv)");
    } else {
      paths_ = path::find(dirname, "*.(bgl|tsv)");
    }
    ready();
  }
  std::pair<GraphType&, const path&> operator*() { return {g_, paths_[index_]}; };
  graph_folder_iterator &operator++() { ++index_; return ready(); }
  graph_folder_iterator &begin() { return *this; }
  graph_folder_iterator end() const { return {}; }
  bool operator!=(const graph_folder_iterator &rhs) const {
    return index_ != paths_.size() || rhs.index_ != rhs.paths_.size();
  }

private:
  std::size_t index_;
  bool rename_id_;
  std::vector<path> paths_;
  GraphType g_;

  graph_folder_iterator &ready() {
    for (; index_ < paths_.size(); ++index_) {
      const path &p = paths_[index_];
      std::optional<GraphType> gopt;
      if (p.extension() == ".bgl") {
        gopt = read_graph_binary_optional<GraphType>(p, true);
      }
      if (p.extension() == ".tsv") {
        gopt = read_graph_tsv_optional<GraphType>(p, rename_id_, true);
      }
      if (gopt.has_value()) {
        g_ = std::move(gopt.value());
        break;
      }
    }
    return *this;
  }
};

} // namespace bgl
