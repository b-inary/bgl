#include "../extlib/catch.hpp"
#include "bgl/graph/analysis/connectivity.hpp"
using namespace bgl;

TEST_CASE("connectivity", "[analysis]") {
  unweighted_adjacency_list adj = {{1}, {2}, {0}, {1, 2, 5}, {2, 6}, {3, 4}, {4}, {5, 6}};
  graph g = adj;
  REQUIRE(is_connected(g));
  REQUIRE(strongly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{4, {0, 0, 0, 2, 1, 2, 1, 3}});

  g.add_edge(6, 7);
  g.remove_edge(5, 3);
  REQUIRE(strongly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{3, {0, 0, 0, 2, 1, 1, 1, 1}});

  g.remove_edge(7, 5);
  REQUIRE(is_connected(g));
  REQUIRE(strongly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{4, {0, 0, 0, 3, 1, 2, 1, 1}});

  g.add_edge(2, 3);
  REQUIRE(is_strongly_connected(g));

  g.remove_edge(3, 5);
  g.remove_edge(4, 2);
  REQUIRE(weakly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{2, {0, 0, 0, 0, 1, 1, 1, 1}});
  REQUIRE(strongly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{3, {0, 0, 0, 0, 1, 2, 1, 1}});

  g.remove_edge(1, 2);
  REQUIRE(weakly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{2, {0, 0, 0, 0, 1, 1, 1, 1}});
  REQUIRE(strongly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{5, {1, 0, 2, 2, 3, 4, 3, 3}});

  g.remove_edge(0, 1);
  g.remove_edge(3, 1);
  REQUIRE(weakly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{3, {0, 1, 0, 0, 2, 2, 2, 2}});
  REQUIRE(strongly_connected_components(g) ==
          std::pair<node_t, std::vector<node_t>>{5, {0, 1, 2, 2, 3, 4, 3, 3}});

  extract_largest_wcc(g);
  REQUIRE(g.num_nodes() == 4);
  REQUIRE(g.num_edges() == 5);

  extract_largest_scc(g);
  REQUIRE(g.num_nodes() == 3);
  REQUIRE(g.num_edges() == 4);
}
