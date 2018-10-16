#include "extlib/catch.hpp"
#include "visitor.hpp"
#include "generator/all.hpp"
using namespace bgl;

TEST_CASE("visitor-unweighted", "[visitor]") {
  unweighted_edge_list es = {{0, 1}, {1, 2}, {2, 3}, {3, 1}};
  graph g1 = es;
  int inf = std::numeric_limits<int>::max();

  auto ds1 = single_source_distance(g1, 0);
  REQUIRE(ds1 == std::vector<int>{0, 1, 2, 3});
  auto ds2 = single_source_distance(g1, 1);
  REQUIRE(ds2 == std::vector<int>{inf, 0, 1, 2});

  g1.make_undirected();
  auto ds3 = single_source_distance(g1, 0);
  REQUIRE(ds3 == std::vector<int>{0, 1, 2, 2});
  auto ds4 = single_source_distance(g1, 1);
  REQUIRE(ds4 == std::vector<int>{1, 0, 1, 1});

  graph g2 = gen::grid(3, 3);
  auto ds5 = single_source_distance(g2, 0);
  REQUIRE(ds5 == std::vector<int>{0, 1, 2, 1, 2, 3, 2, 3, 4});
}

TEST_CASE("visitor-weighted", "[visitor]") {
  weighted_edge_list<double> es = {{0, {1, 1.5}}, {1, {2, 20}}, {2, {3, 5}}, {3, {1, 0.25}}};
  wgraph<double> g = es;
  double inf = std::numeric_limits<double>::max();

  auto ds1 = single_source_distance(g, 0);
  REQUIRE(ds1 == std::vector<double>{0, 1.5, 21.5, 26.5});
  auto ds2 = single_source_distance(g, 1);
  REQUIRE(ds2 == std::vector<double>{inf, 0, 20, 25});

  g.make_undirected();
  auto ds3 = single_source_distance(g, 0);
  REQUIRE(ds3 == std::vector<double>{0, 1.5, 6.75, 1.75});
  auto ds4 = single_source_distance(g, 1);
  REQUIRE(ds4 == std::vector<double>{1.5, 0, 5.25, 0.25});
}
