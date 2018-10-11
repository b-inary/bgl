#pragma once
#include "util/all.hpp"
#include "basic_graph.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <filesystem>

// istream support of weighted_edge_t
namespace std {
template <typename WeightType>
istream &operator<<(istream &is, bgl::weighted_edge_t<WeightType> &e) {
  return is >> e.first >> e.second;
}
} // namespace std

namespace bgl {

/* tsv I/O */

//! write unweighted edge to tsv file
inline void write_edge_tsv(std::ostream &os, const unweighted_edge_t &e) {
  fmt::print(os, "{}", to(e));
}

//! write weighted edge to tsv file
template <typename WeightType>
void write_edge_tsv(std::ostream &os, const weighted_edge_t<WeightType> &e) {
  fmt::print(os, "{} {}", to(e), weight(e));
}

//! write graph as tsv file to |os|
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

//! write graph as tsv file to |filename|
template <typename GraphType>
void write_graph_tsv(std::filesystem::path file, const GraphType &g, bool write_info = true) {
  std::ofstream ofs(file);
  require_msg(ofs, "write_graph_tsv: file cannot open: {}", file.string());
  write_graph_tsv(ofs, g, write_info);
}

//! read graph from |is| as tsv file
template <typename GraphType>
GraphType read_graph_tsv(std::istream &is, bool canonicalize = true) {
  using edge_t = typename GraphType::edge_type;
  require_msg(is, "read_graph_tsv: empty stream");

  std::string line;
  edge_list<edge_t> es;
  node_t num_v = 0;
  std::unordered_map<size_t, node_t> id;

  for (std::size_t lineno = 1; std::getline(is, line); ++lineno) {
    size_t v;
    edge_t e;
    if (line.empty() || line[0] == '#') continue;
    std::stringstream ss(line);
    require_msg(ss >> v >> e, "read_graph_tsv: read failed at line {}\n  read: {}\n  type: {}",
                lineno, line, typename_of(GraphType{}));
    if (canonicalize) {
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
GraphType read_graph_tsv(std::filesystem::path file, bool canonicalize = true) {
  std::ifstream ifs(file);
  require_msg(ifs, "read_graph_tsv: file not exist: {}", file.string());
  return read_graph_tsv<GraphType>(ifs, canonicalize);
}
} // namespace bgl
