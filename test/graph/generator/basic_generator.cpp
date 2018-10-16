#include "extlib/catch.hpp"
#include "bgl/graph/generator/basic_generator.hpp"
using namespace bgl;

TEST_CASE("generators") {
  SECTION("complete") {
    graph g = gen::complete(10);
    REQUIRE(g.num_nodes() == 10);
    REQUIRE(g.num_edges() == 90);
    REQUIRE(g.is_adjacent(3, 7));
    REQUIRE(g.is_adjacent(7, 3));
    REQUIRE(g.is_adjacent(9, 2));
  }

  SECTION("bipartite") {
    graph g = gen::dir_complete_bipartite(3, 5);
    REQUIRE(g.num_nodes() == 8);
    REQUIRE(g.num_edges() == 15);
    REQUIRE(g.is_adjacent(0, 3));
    REQUIRE(g.is_adjacent(1, 5));
    REQUIRE(g.is_adjacent(2, 7));

    g.make_undirected();
    REQUIRE(g.num_edges() == 30);
    REQUIRE(g.is_adjacent(3, 0));
    REQUIRE(g.is_adjacent(5, 1));
    REQUIRE(g.is_adjacent(7, 2));
  }

  SECTION("star") {
    graph g = gen::star(1);
    REQUIRE(g.num_nodes() == 1);
    REQUIRE(g.num_edges() == 0);

    g = gen::star(10);
    REQUIRE(g.num_nodes() == 10);
    REQUIRE(g.num_edges() == 18);
    REQUIRE(g.is_adjacent(0, 3));
    REQUIRE(g.is_adjacent(0, 5));
    REQUIRE(g.is_adjacent(7, 0));
  }

  SECTION("path") {
    graph g = gen::path(10);
    REQUIRE(g.num_nodes() == 10);
    REQUIRE(g.num_edges() == 18);
    REQUIRE(g.is_adjacent(0, 1));
    REQUIRE(g.is_adjacent(7, 8));
    REQUIRE(g.is_adjacent(8, 7));
  }

  SECTION("grid") {
    graph g = gen::grid(3, 4);
    REQUIRE(g.num_nodes() == 12);
    REQUIRE(g.num_edges() == ((2 * 3 * 4 - 3 - 4) * 2));
    REQUIRE(g.outdegree(0) == 2);
    REQUIRE(g.outdegree(1) == 3);
    REQUIRE(g.outdegree(2) == 2);
    REQUIRE(g.outdegree(3) == 3);
    REQUIRE(g.outdegree(4) == 4);
    REQUIRE(g.is_adjacent(0, 1));
    REQUIRE(g.is_adjacent(0, 3));
    REQUIRE(g.is_adjacent(4, 1));
  }

  SECTION("grid-3d") {
    graph g = gen::grid_3d(3, 4, 5);
    REQUIRE(g.num_nodes() == (3 * 4 * 5));
    REQUIRE(g.num_edges() == ((3 * 3 * 4 * 5 - 3 * 4 - 4 * 5 - 5 * 3) * 2));
    REQUIRE(g.outdegree(0) == 3);
    REQUIRE(g.outdegree(1) == 4);
    REQUIRE(g.outdegree(2) == 3);
    REQUIRE(g.outdegree(3) == 4);
    REQUIRE(g.outdegree(4) == 5);
    REQUIRE(g.outdegree(12) == 4);
    REQUIRE(g.outdegree(13) == 5);
    REQUIRE(g.outdegree(14) == 4);
    REQUIRE(g.outdegree(15) == 5);
    REQUIRE(g.outdegree(16) == 6);
  }

  SECTION("cycle") {
    graph g = gen::dir_cycle(1);
    REQUIRE(g.num_nodes() == 1);
    REQUIRE(g.num_edges() == 0);

    g = gen::dir_cycle(2);
    REQUIRE(g.num_nodes() == 2);
    REQUIRE(g.num_edges() == 2);
    g.make_undirected();
    REQUIRE(g.num_edges() == 2);

    g = gen::dir_cycle(10);
    REQUIRE(g.num_nodes() == 10);
    REQUIRE(g.num_edges() == 10);
    REQUIRE(g.is_adjacent(0, 1));
    REQUIRE(g.is_adjacent(3, 4));
    REQUIRE(g.is_adjacent(9, 0));
    g.make_undirected();
    REQUIRE(g.num_edges() == 20);
    REQUIRE(g.is_adjacent(0, 9));
  }
}
