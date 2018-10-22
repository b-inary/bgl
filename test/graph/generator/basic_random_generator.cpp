#include "extlib/catch.hpp"
#include "bgl/graph/io.hpp"
#include "bgl/graph/generator/basic_generator.hpp"
#include "bgl/graph/generator/basic_random_generator.hpp"
using namespace bgl;

TEST_CASE("erdos-renyi", "[generator]") {
  graph g = gen::erdos_renyi(1, 0.5);
  REQUIRE(g.num_nodes() == 1);
  REQUIRE(g.num_edges() == 0);

  g = gen::erdos_renyi(100, 20);
  REQUIRE(g.num_edges() > 18 * 100);
  REQUIRE(g.num_edges() < 22 * 100);

  g = gen::erdos_renyi(100, 50);
  REQUIRE(g.num_edges() > 48 * 100);
  REQUIRE(g.num_edges() < 52 * 100);
}

TEST_CASE("configuration", "[generator]") {
  graph g = gen::grid(5, 5);
  graph gconf = gen::configuration(g);
  REQUIRE(g.outdegree(0) == gconf.outdegree(0));
  REQUIRE(g.num_edges() == gconf.num_edges());
}

TEST_CASE("configuration-2d", "[generator]") {
  graph g = gen::grid(5, 5);
  graph gconf = gen::configuration_2d(g);
  REQUIRE(g.outdegree(0) == gconf.outdegree(0));
  REQUIRE(g.num_edges() == gconf.num_edges());
}
