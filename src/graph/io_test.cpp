#include "extlib/catch.hpp"
#include "io.hpp"
#include <iostream>
using namespace bgl;

TEST_CASE("graph I/O", "[io]") {
  graph g = read_graph_tsv<graph>("datasets/karate.tsv", false);
  g.pretty_print();
  REQUIRE(g.num_nodes() == 34);
  REQUIRE(g.num_edges() == 156);

  std::ofstream ofs("datasets/karate.out.tsv");
  write_graph_tsv(ofs, g);
  graph g2 = read_graph_tsv<graph>("datasets/karate.out.tsv", false);
  REQUIRE(g == g2);
}
