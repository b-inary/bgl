#include "../extlib/catch.hpp"
#include "bgl/graph/io.hpp"
#include <iostream>
using namespace bgl;

TEST_CASE("graph I/O", "[graph-io]") {
  graph g = read_graph<graph>("datasets/karate.tsv");
  REQUIRE(g.num_nodes() == 34);
  REQUIRE(g.num_edges() == 156);

  graph g2 = read_graph<graph>("datasets/karate.tsv.zst");
  REQUIRE(g == g2);

  write_graph_tsv("datasets/karate.out.tsv", g);
  graph g3 = read_graph<graph>("datasets/karate.out.tsv");
  REQUIRE(g == g3);

  write_graph_binary("datasets/karate.out.bgl", g);
  graph g4 = read_graph<graph>("datasets/karate.out.bgl");
  REQUIRE(g == g4);

  path::remove("datasets/karate.out.tsv");
  path::remove("datasets/karate.out.bgl");
}
