#include "../extlib/catch.hpp"
#include "bgl/graph/basic_graph.hpp"
#include <algorithm>
#include <vector>
using namespace bgl;

TEST_CASE("unweighted graph", "[basic-graph]") {
  unweighted_edge_list es = {{0, 1}, {1, 2}, {2, 3}, {3, 1}};
  graph g = es;

  SECTION("basic") {
    REQUIRE(g.num_nodes() == 4);
    REQUIRE(g.num_edges() == 4);
    REQUIRE(g.outdegree(1) == 1);

    std::vector<node_t> nodes = {0, 1, 2, 3};
    REQUIRE(std::equal(g.nodes().begin(), g.nodes().end(), nodes.begin()));

    std::vector<node_t> edges_1 = {2};
    REQUIRE(std::equal(g.edges(1).begin(), g.edges(1).end(), edges_1.begin()));
    REQUIRE(std::equal(g.neighbors(1).begin(), g.neighbors(1).end(), edges_1.begin()));

    REQUIRE(g.is_adjacent(0, 2) == false);
    REQUIRE(g.is_adjacent(2, 3) == true);
    REQUIRE(g.is_adjacent(3, 2) == false);

    REQUIRE(g.get_edge_list() == es);

    unweighted_adjacency_list adj = {{1}, {2}, {3}, {1}};
    graph g2 = adj;
    REQUIRE(g == g2);

    graph g3 = g2;
    g3.add_edge(0, 2);
    REQUIRE(g2.num_edges() == 4);
    REQUIRE(g3.num_edges() == 5);
    REQUIRE(g2.outdegree(0) == 1);
    REQUIRE(g3.outdegree(0) == 2);
    std::swap(g2, g3);
    REQUIRE(g2.num_edges() == 5);
    REQUIRE(g3.num_edges() == 4);
    REQUIRE(g2.outdegree(0) == 2);
    REQUIRE(g3.outdegree(0) == 1);
  }

  SECTION("simplify") {
    graph g2 = g;
    g2.simplify();
    REQUIRE(g2.num_edges() == 4);

    unweighted_edge_list es2 = {{0, 1}, {1, 2}, {2, 3}, {3, 1}, {1, 2}};
    g2 = es2;
    REQUIRE(g2.outdegree(1) == 2);
    REQUIRE(g2.num_edges() == 5);
    g2.simplify();
    REQUIRE(g2.outdegree(1) == 1);
    REQUIRE(g2.num_edges() == 4);
  }

  SECTION("transpose") {
    graph g2 = g;
    g2.transpose();
    REQUIRE(g2.is_adjacent(0, 2) == false);
    REQUIRE(g2.is_adjacent(2, 3) == false);
    REQUIRE(g2.is_adjacent(3, 2) == true);
  }

  SECTION("make_undirected") {
    graph g2 = g;
    g2.make_undirected();
    REQUIRE(g2.num_edges() == 8);
    REQUIRE(g2.is_adjacent(0, 2) == false);
    REQUIRE(g2.is_adjacent(2, 3) == true);
    REQUIRE(g2.is_adjacent(3, 2) == true);

    unweighted_edge_list es2 = {{0, 1}, {1, 2}, {2, 1}, {2, 3}, {3, 1}};
    g2 = es2;
    g2.make_undirected();
    REQUIRE(g2.num_edges() == 8);
  }

  SECTION("remove isolated nodes") {
    graph g2 = g;
    g2.remove_isolated_nodes();
    REQUIRE(g2.num_nodes() == 4);
    g2.remove_edge(0, 1);
    REQUIRE(g2.num_edges() == 3);
    g2.remove_isolated_nodes();
    REQUIRE(g2.is_adjacent(0, 1) == true);
    REQUIRE(g2.is_adjacent(1, 2) == true);
    REQUIRE(g2.is_adjacent(2, 0) == true);
    REQUIRE(g2.is_adjacent(0, 2) == false);
  }
}

TEST_CASE("weighted graph", "[basic-graph]") {
  weighted_edge_list<int> es = {{0, {1, 1}}, {1, {2, 2}}, {2, {3, 3}}, {3, {1, 4}}};
  wgraph<int> g = es;

  SECTION("basic") {
    REQUIRE(g.num_nodes() == 4);
    REQUIRE(g.num_edges() == 4);
    REQUIRE(g.outdegree(1) == 1);

    std::vector<node_t> nodes = {0, 1, 2, 3};
    REQUIRE(std::equal(g.nodes().begin(), g.nodes().end(), nodes.begin()));

    std::vector<weighted_edge_t<int>> edges_1 = {{2, 2}};
    REQUIRE(std::equal(g.edges(1).begin(), g.edges(1).end(), edges_1.begin()));

    std::vector<node_t> neighbors_1 = {2};
    REQUIRE(std::equal(g.neighbors(1).begin(), g.neighbors(1).end(), neighbors_1.begin()));

    REQUIRE(g.get_weight(0, 2).value_or(0) == 0);
    REQUIRE(g.get_weight(2, 3).value_or(0) == 3);
    REQUIRE(g.get_weight(3, 2).value_or(0) == 0);

    REQUIRE(g.get_edge_list() == es);

    weighted_adjacency_list<int> adj = {{{1, 1}}, {{2, 2}}, {{3, 3}}, {{1, 4}}};
    wgraph<int> g2 = adj;
    REQUIRE(g == g2);
  }

  SECTION("simplify") {
    wgraph<int> g2 = g;
    g2.simplify();
    REQUIRE(g2.num_edges() == 4);

    weighted_edge_list<int> es2 = {{0, {1, 1}}, {1, {2, 2}}, {2, {3, 3}}, {3, {1, 4}}, {1, {2, 2}}};
    g2 = es2;
    REQUIRE(g2.outdegree(1) == 2);
    REQUIRE(g2.num_edges() == 5);
    g2.simplify();
    REQUIRE(g2.outdegree(1) == 1);
    REQUIRE(g2.num_edges() == 4);

    weighted_edge_list<int> es3 = {{0, {1, 1}}, {1, {2, 2}}, {2, {3, 3}}, {3, {1, 4}}, {1, {2, 3}}};
    g2 = es3;
    REQUIRE(g2.outdegree(1) == 2);
    REQUIRE(g2.num_edges() == 5);
    g2.simplify(true);
    REQUIRE(g2.outdegree(1) == 2);
    REQUIRE(g2.num_edges() == 5);
    g2.simplify();
    REQUIRE(g2.outdegree(1) == 1);
    REQUIRE(g2.num_edges() == 4);
  }

  SECTION("transpose") {
    wgraph<int> g2 = g;
    g2.transpose();
    REQUIRE(g2.get_weight(0, 2).value_or(0) == 0);
    REQUIRE(g2.get_weight(2, 3).value_or(0) == 0);
    REQUIRE(g2.get_weight(3, 2).value_or(0) == 3);
  }

  SECTION("make_undirected") {
    wgraph<int> g2 = g;
    g2.make_undirected();
    REQUIRE(g2.num_edges() == 8);
    REQUIRE(g2.get_weight(0, 2).value_or(0) == 0);
    REQUIRE(g2.get_weight(2, 3).value_or(0) == 3);
    REQUIRE(g2.get_weight(3, 2).value_or(0) == 3);

    weighted_edge_list<int> es2 = {{0, {1, 1}}, {1, {2, 2}}, {2, {3, 3}}, {2, {1, 2}}, {3, {1, 4}}};
    g2 = es2;
    g2.make_undirected();
    REQUIRE(g2.num_edges() == 8);

    weighted_edge_list<int> es3 = {{0, {1, 1}}, {1, {2, 2}}, {2, {3, 3}}, {2, {1, 3}}, {3, {1, 4}}};
    g2 = es3;
    g2.make_undirected();
    REQUIRE(g2.num_edges() == 10);
  }

  SECTION("permutation") {
    wgraph<int> g2 = g;
    g2.permute_nodes({2, 3, 1, 0});
    REQUIRE(g2.edge(0, 0) == weighted_edge_t<int>{1, 3});
    REQUIRE(g2.edge(1, 0) == weighted_edge_t<int>{2, 4});
    REQUIRE(g2.edge(2, 0) == weighted_edge_t<int>{0, 2});
    REQUIRE(g2.edge(3, 0) == weighted_edge_t<int>{2, 1});
  }

  SECTION("dynamic update") {
    wgraph<int> g2 = g;
    g2.add_edge(2, {3, 1});
    REQUIRE(g2.num_edges() == 5);
    REQUIRE(g2.get_weight(2, 3).value_or(0) == 1);
    g2.remove_edge(2, 3);
    REQUIRE(g2.num_edges() == 3);
    REQUIRE(g2.is_adjacent(2, 3) == false);
  }
}
