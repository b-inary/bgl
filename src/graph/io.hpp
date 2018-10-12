#pragma once
#include "util/all.hpp"
#include "basic_graph.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <type_traits>
#include <cstdint>

// istream support of weighted_edge_t
namespace std {
template <typename WeightType>
istream &operator<<(istream &is, bgl::weighted_edge_t<WeightType> &e) {
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

//! read graph from |is| as tsv file
template <typename GraphType>
GraphType read_graph_tsv(std::istream &is, bool rename_id = false) {
  using edge_t = typename GraphType::edge_type;
  require_msg(is, "read_graph_tsv: empty stream");

  std::string line;
  edge_list<edge_t> es;
  node_t num_v = 0;
  std::unordered_map<std::size_t, node_t> id;

  for (std::size_t lineno = 1; std::getline(is, line); ++lineno) {
    std::size_t v;
    edge_t e;
    if (line.empty() || line[0] == '#') continue;
    std::stringstream ss(line);
    require_msg(ss >> v >> e, "read_graph_tsv: read failed at line {}\n  read: {}\n  type: {}",
                lineno, line, typename_of(GraphType{}));
    if (rename_id) {
      if (id.count(v) == 0) id[v] = num_v++;
      if (id.count(to(e)) == 0) id[to(e)] = num_v++;
      v = id[v];
      update_to(e, id[to(e)]);
    }
    es.emplace_back(static_cast<node_t>(v), e);
  }

  return {es};
}

//! read graph from |filename| as tsv file
template <typename GraphType>
GraphType read_graph_tsv(path file, bool rename_id = false) {
  std::ifstream ifs(file.string());
  require_msg(ifs, "read_graph_tsv: file not exist: {}", file);
  return read_graph_tsv<GraphType>(ifs, rename_id);
}

//! write graph to |os| as tsv file
template <typename GraphType>
void write_graph_tsv(std::ostream &os, const GraphType &g, bool write_info = true) {
  require_msg(os, "write_graph_tsv: empty stream");
  if (write_info) {
    fmt::print(os, "# number of nodes: {}\n", g.num_nodes());
    fmt::print(os, "# number of edges: {}\n", g.num_edges());
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

//! write graph to |filename| as tsv file
template <typename GraphType>
void write_graph_tsv(path filename, const GraphType &g, bool write_info = true) {
  std::ofstream ofs(filename.string());
  require_msg(ofs, "write_graph_tsv: file cannot open: {}", filename);
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

inline std::size_t sizeof_edge_weight(const graph &g [[maybe_unused]]) {
  return 0;
}

template <typename WeightType>
std::size_t sizeof_edge_weight(const wgraph<WeightType> &g [[maybe_unused]]) {
  return sizeof(WeightType);
}

//! read graph in binary format from |is|
template <typename GraphType>
GraphType read_graph_binary(std::istream &is) {
  using edge_t = typename GraphType::edge_type;
  using weight_t = decltype(weight(edge_t{}));
  static_assert(std::is_arithmetic_v<weight_t>, "edge weight must be arithmetic type");
  require_msg(is, "read_graph_binary: empty stream");

  // check header
  char buf[4];
  is.read(buf, 4);
  require_msg(
    is.gcount() == 4 && buf[0] == 'b' && buf[1] == 'g' && buf[2] == 'l' && buf[3] == '\0',
    "read_graph_binary: invalid header"
  );

  std::uint32_t edge_size = read_binary<std::uint32_t>(is);
  bool is_integral = read_binary<std::uint32_t>(is);
  require_msg(
    edge_size == sizeof_edge_weight(GraphType{}) && is_integral == std::is_integral_v<weight_t>,
    "read_graph_binary: edge type does not match\n  read type: {}\n"
    "  edge weight information: size = {} bytes, is_integral = {}",
    typename_of(GraphType{}), edge_size, is_integral
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

  return {num_nodes, num_edges, std::move(adj)};
}

//! read graph from |filename| in binary format
template <typename GraphType>
GraphType read_graph_binary(path filename) {
  std::ifstream ifs(filename.string(), std::ios_base::binary);
  require_msg(ifs, "read_graph_binary: file not exist: {}", filename);
  return read_graph_binary<GraphType>(ifs);
}

//! write graph to |os| in binary format
template <typename GraphType>
void write_graph_binary(std::ostream &os, const GraphType &g) {
  using edge_t = typename GraphType::edge_type;
  using weight_t = decltype(weight(edge_t{}));
  static_assert(std::is_arithmetic_v<weight_t>, "edge weight must be arithmetic type");
  require_msg(os, "write_graph_binary: empty stream");

  // header
  os.write("bgl", 4);
  write_binary(os, static_cast<std::uint32_t>(sizeof_edge_weight(g)));
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

//! write graph to |filename| in binary format
template <typename GraphType>
void write_graph_binary(path filename, const GraphType &g) {
  std::ofstream ofs(filename.string(), std::ios_base::binary);
  require_msg(ofs, "write_graph_binary: file cannot open: {}", filename);
  write_graph_binary(ofs, g);
}
} // namespace bgl
