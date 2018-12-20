#pragma once
#include "basic_graph.hpp"
#include "bgl/util/all.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <type_traits>
#include <utility>

// istream support of weighted_edge_t
namespace std {
template <typename WeightType>
istream &operator>>(istream &is, bgl::weighted_edge_t<WeightType> &e) {
  return is >> e.first >> e.second;
}
}  // namespace std

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
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_tsv_optional(std::istream &is, bool accept_mismatch = false) {
  using edge_t = typename GraphType::edge_type;
  ASSERT_MSG(is, "{}: empty stream", __func__);

  std::string line;
  edge_list<edge_t> es;
  bool type_checked = false;

  for (std::size_t lineno = 1; std::getline(is, line); ++lineno) {
    node_t v;
    edge_t e;

    // weight type check
    static const std::string type_comment = "# weight type: ";
    if (line.compare(0, type_comment.size(), type_comment) == 0) {
      std::string type_string = line.substr(type_comment.size());
      std::string read_as = GraphType{}.weight_string();
      if (accept_mismatch && type_string != read_as) {
        return std::nullopt;
      }
      ASSERT_MSG(type_string == read_as,
                 "{}: type of edge weight does not match\n  read as: {}\n  input type: {}",
                 __func__, read_as, type_string);
      type_checked = true;
    }

    if (line.empty() || line[0] == '#') continue;

    std::stringstream ss(line);
    bool read_success = static_cast<bool>(ss >> v >> e);
    if (!read_success && accept_mismatch && !type_checked) {
      return std::nullopt;
    }

    ASSERT_MSG(read_success, "{}: read failed at line {}\n  read: {}\n  weight type: {}", __func__,
               lineno, line, GraphType{}.weight_string());

    es.emplace_back(v, e);
  }

  return GraphType(es);
}

/// read graph from |filename| as tsv file
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_tsv_optional(const path &file, bool accept_mismatch = false) {
  std::ifstream ifs(file.string());
  ASSERT_MSG(ifs, "{}: file does not exist: {}", __func__, file);
  return read_graph_tsv_optional<GraphType>(ifs, accept_mismatch);
}

/// read graph from |is| as tsv file
template <typename GraphType>
GraphType read_graph_tsv(std::istream &is) {
  return read_graph_tsv_optional<GraphType>(is).value();
}

/// read graph from |filename| as tsv file
template <typename GraphType>
GraphType read_graph_tsv(const path &filename) {
  return read_graph_tsv_optional<GraphType>(filename).value();
}

/// write graph to |os| as tsv file
template <typename GraphType>
void write_graph_tsv(std::ostream &os, const GraphType &g, bool write_info = true) {
  ASSERT_MSG(os, "{}: empty stream", __func__);
  if (write_info) {
    fmt::print(os, "# number of nodes: {}\n", g.num_nodes());
    fmt::print(os, "# number of edges: {}\n", g.num_edges());
    fmt::print(os, "# weight type: {}\n", g.weight_string());
  }
  for (node_t v : g.nodes()) {
    for (const auto &e : g.edges(v)) {
      fmt::print(os, "{} ", v);
      write_edge_tsv(os, e);
      fmt::print(os, "\n");
    }
  }
  os.flush();
}

/// write graph to |filename| as tsv file
template <typename GraphType>
void write_graph_tsv(const path &filename, const GraphType &g, bool write_info = true) {
  std::ofstream ofs(filename.string());
  ASSERT_MSG(ofs, "{}: file cannot open: {}", __func__, filename);
  write_graph_tsv(ofs, g, write_info);
}


/* binary I/O */

/*
 *  binary format specification:
 *    - 4 bytes: magic constant: "bgl\0"
 *    - 4 bytes: weight size [byte]
 *    - 4 bytes: whether weight type is integral (0 or 1; 1 when unweighted)
 *    - 4 bytes: the number of nodes
 *    - 8 bytes: the number of edges
 *    + repeat (# of nodes) times:
 *        - 8 bytes: outdegree of node
 *        + repeat (outdegree) times:
 *            - 4 bytes: connected node (sorted)
 *            - (weight size) bytes: edge weight
 */

template <typename T>
T read_binary(std::istream &is) {
  T t{};
  is.read(reinterpret_cast<char *>(&t), sizeof(T));
  return t;
}

template <typename T>
void write_binary(std::ostream &os, const T &t) {
  os.write(reinterpret_cast<const char *>(&t), sizeof(T));
}

/// read graph from |is| in binary format .
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_binary_optional(std::istream &is,
                                                    bool accept_mismatch = false) {
  using edge_t = typename GraphType::edge_type;
  using weight_t = decltype(weight(edge_t{}));
  static_assert(std::is_arithmetic_v<weight_t>, "edge weight must be arithmetic type");
  ASSERT_MSG(is, "{}: empty stream", __func__);

  // check header
  char buf[4];
  is.read(buf, 4);
  ASSERT_MSG(is.gcount() == 4 && buf[0] == 'b' && buf[1] == 'g' && buf[2] == 'l' && buf[3] == '\0',
             "{}: invalid header", __func__);

  // check weight type
  std::uint32_t edge_size = read_binary<std::uint32_t>(is);
  bool is_integral = read_binary<std::uint32_t>(is);
  bool type_matched =
      edge_size == GraphType{}.weight_sizeof() && is_integral == std::is_integral_v<weight_t>;

  if (accept_mismatch && !type_matched) {
    return std::nullopt;
  }

  ASSERT_MSG(type_matched,
             "{}: type of edge weight does not match\n"
             "  read as: {}\n  input type: size = {} byte(s), is_integral = {}",
             __func__, GraphType{}.weight_string(), edge_size, is_integral);

  // graph body
  node_t num_nodes = read_binary<node_t>(is);
  std::uint64_t num_edges = read_binary<std::uint64_t>(is);
  adjacency_list<edge_t> adj(num_nodes);
  for (node_t v : irange(num_nodes)) {
    std::uint64_t degree = read_binary<std::uint64_t>(is);
    adj[v].resize(degree);
    is.read(reinterpret_cast<char *>(adj[v].data()), degree * sizeof(edge_t));
  }

  is.peek();
  ASSERT_MSG(is.eof() && !is.fail(), "{}: read failed (invalid bgl file)", __func__);

  return GraphType(num_nodes, num_edges, std::move(adj));
}

/// read graph from |filename| in binary format.
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_binary_optional(const path &filename,
                                                    bool accept_mismatch = false) {
  std::ifstream ifs(filename.string(), std::ios_base::binary);
  ASSERT_MSG(ifs, "{}: file does not exist: {}", __func__, filename);
  return read_graph_binary_optional<GraphType>(ifs, accept_mismatch);
}

/// read graph from |is| in binary format
template <typename GraphType>
GraphType read_graph_binary(std::istream &is) {
  return read_graph_binary_optional<GraphType>(is).value();
}

/// read graph from |filename| in binary format
template <typename GraphType>
GraphType read_graph_binary(const path &filename) {
  return read_graph_binary_optional<GraphType>(filename).value();
}

/// write graph to |os| in binary format
template <typename GraphType>
void write_graph_binary(std::ostream &os, const GraphType &g) {
  using edge_t = typename GraphType::edge_type;
  using weight_t = decltype(weight(edge_t{}));
  static_assert(std::is_arithmetic_v<weight_t>, "edge weight must be arithmetic type");
  ASSERT_MSG(os, "{}: empty stream", __func__);

  // header
  os.write("bgl", 4);
  write_binary(os, static_cast<std::uint32_t>(g.weight_sizeof()));
  write_binary(os, static_cast<std::uint32_t>(std::is_integral_v<weight_t>));

  // graph body
  write_binary(os, g.num_nodes());
  write_binary(os, static_cast<std::uint64_t>(g.num_edges()));
  for (node_t v : g.nodes()) {
    write_binary(os, static_cast<std::uint64_t>(g.outdegree(v)));
    os.write(reinterpret_cast<const char *>(g.edges(v).data()), g.outdegree(v) * sizeof(edge_t));
  }

  os.flush();
}

/// write graph to |filename| in binary format
template <typename GraphType>
void write_graph_binary(const path &filename, const GraphType &g) {
  std::ofstream ofs(filename.string(), std::ios_base::binary);
  ASSERT_MSG(ofs, "{}: file cannot open: {}", __func__, filename);
  write_graph_binary(ofs, g);
}


/* compressed input */

/// read compressed tsv graph from |is|
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_tsv_zstd_optional(std::istream &is,
                                                      bool accept_mismatch = false) {
  zstd_decode_filter_buf buf(is.rdbuf());
  std::istream is_decoded(&buf);
  return read_graph_tsv_optional<GraphType>(is_decoded, accept_mismatch);
}

/// read comressed tsv graph from |filename|
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_tsv_zstd_optional(const path &file,
                                                      bool accept_mismatch = false) {
  std::ifstream ifs(file.string(), std::ios_base::binary);
  ASSERT_MSG(ifs, "{}: file does not exist: {}", __func__, file);
  return read_graph_tsv_zstd_optional<GraphType>(ifs, accept_mismatch);
}

/// read compressed tsv graph from |is|
template <typename GraphType>
GraphType read_graph_tsv_zstd(std::istream &is) {
  return read_graph_tsv_zstd_optional<GraphType>(is).value();
}

/// read compressed tsv graph from |filename|
template <typename GraphType>
GraphType read_graph_tsv_zstd(const path &filename) {
  return read_graph_tsv_zstd_optional<GraphType>(filename).value();
}

/// read compressed binary graph from |is|
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_binary_zstd_optional(std::istream &is,
                                                         bool accept_mismatch = false) {
  zstd_decode_filter_buf buf(is.rdbuf());
  std::istream is_decoded(&buf);
  return read_graph_binary_optional<GraphType>(is_decoded, accept_mismatch);
}

/// read compressed binary graph from |filename|
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_binary_zstd_optional(const path &filename,
                                                         bool accept_mismatch = false) {
  std::ifstream ifs(filename.string(), std::ios_base::binary);
  ASSERT_MSG(ifs, "{}: file does not exist: {}", __func__, filename);
  return read_graph_binary_zstd_optional<GraphType>(ifs, accept_mismatch);
}

/// read compressed binary graph from |is|
template <typename GraphType>
GraphType read_graph_binary_zstd(std::istream &is) {
  return read_graph_binary_zstd_optional<GraphType>(is).value();
}

/// read compressed binary graph from |filename|
template <typename GraphType>
GraphType read_graph_binary_zstd(const path &filename) {
  return read_graph_binary_zstd_optional<GraphType>(filename).value();
}

/// read graph in automatically determined format
/// when type does not match and |accept_mismatch|, return |nullopt|
template <typename GraphType>
std::optional<GraphType> read_graph_optional(const path &filename, bool accept_mismatch = false) {
  std::string ext = filename.extension();
  if (ext == ".zst") {
    std::string second_ext = filename.clone().replace_extension().extension();
    if (second_ext == ".bgl") {
      return read_graph_binary_zstd_optional<GraphType>(filename, accept_mismatch);
    } else {
      return read_graph_tsv_zstd_optional<GraphType>(filename, accept_mismatch);
    }
  } else if (ext == ".bgl") {
    return read_graph_binary_optional<GraphType>(filename, accept_mismatch);
  } else {
    return read_graph_tsv_optional<GraphType>(filename, accept_mismatch);
  }
}

/// read graph in automatially determined format
template <typename GraphType>
GraphType read_graph(const path &filename) {
  return read_graph_optional<GraphType>(filename).value();
}


/* traverse directory */

/// read all graphs whose types are |GraphType|
template <typename GraphType>
class graph_folder_iterator {
public:
  graph_folder_iterator() {}
  graph_folder_iterator(const path &dirname, bool recursive = false) {
    if (recursive) {
      paths_ = path::find_recursive(dirname, "*.(bgl|tsv|zst)");
    } else {
      paths_ = path::find(dirname, "*.(bgl|tsv|zst)");
    }
    ready();
  }

  // prohibit copying
  graph_folder_iterator(const graph_folder_iterator &) = delete;
  graph_folder_iterator(graph_folder_iterator &&) = default;
  graph_folder_iterator &operator=(const graph_folder_iterator &) = delete;
  graph_folder_iterator &operator=(graph_folder_iterator &&) = default;

  graph_folder_iterator &operator++() {
    ++index_;
    return ready();
  }

  std::pair<GraphType &, const path &> operator*() { return {g_, paths_[index_]}; };
  graph_folder_iterator &&begin() { return std::move(*this); }
  graph_folder_iterator end() const { return {}; }
  bool operator!=(const graph_folder_iterator &rhs) const {
    return index_ != paths_.size() || rhs.index_ != rhs.paths_.size();
  }

private:
  std::size_t index_ = 0;
  std::vector<path> paths_;
  GraphType g_;

  graph_folder_iterator &ready() {
    for (; index_ < paths_.size(); ++index_) {
      std::optional<GraphType> gopt = read_graph_optional<GraphType>(paths_[index_], true);
      if (gopt.has_value()) {
        g_ = std::move(gopt.value());
        break;
      }
    }
    return *this;
  }
};

}  // namespace bgl
