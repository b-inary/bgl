#include "extlib/catch.hpp"
#include "io.hpp"
#include <iostream>
using namespace bgl;

TEST_CASE("graph I/O", "[graph-io]") {
  graph g = read_graph_tsv<graph>("datasets/karate.tsv");
  g.pretty_print();
  REQUIRE(g.num_nodes() == 34);
  REQUIRE(g.num_edges() == 156);

  write_graph_tsv("datasets/karate.out.tsv", g);
  graph g2 = read_graph_tsv<graph>("datasets/karate.out.tsv");
  REQUIRE(g == g2);

  write_graph_binary("datasets/karate.out.bgl", g);
  graph g3 = read_graph_binary<graph>("datasets/karate.out.bgl");
  REQUIRE(g == g3);
}
