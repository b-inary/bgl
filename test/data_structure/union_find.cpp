#include "extlib/catch.hpp"
#include "bgl/data_structure/union_find.hpp"
using namespace bgl;

TEST_CASE("union-find", "[data-structure]") {
  union_find uf(10);
  REQUIRE(uf.disjoint_count() == 10);
  REQUIRE(uf.is_same(7, 7) == true);
  REQUIRE(uf.is_same(1, 2) == false);

  uf.unite(2, 9);
  uf.unite(4, 5);
  uf.unite(6, 1);
  uf.unite(5, 2);
  REQUIRE(uf.disjoint_count() == 6);
  REQUIRE(uf.is_same(1, 6) == true);
  REQUIRE(uf.is_same(4, 9) == true);
  REQUIRE(uf.is_same(1, 2) == false);
  REQUIRE(uf.is_same(7, 1) == false);

  uf.unite(9, 5);
  REQUIRE(uf.disjoint_count() == 6);
}
