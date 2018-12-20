#include "../extlib/catch.hpp"
#include "bgl/data_structure/hyperloglog_array.hpp"
#include "bgl/util/all.hpp"
using namespace bgl;

TEST_CASE("hyperloglog", "[data-structure]") {
  SECTION("sequence small") {
    hyperloglog_array h(1, 12);
    REQUIRE(h[0].count() == 0.0);
    for (int i : irange(100)) {
      h[0].insert(i);
    }
    REQUIRE(h[0].count() > 80);
    REQUIRE(h[0].count() < 120);
  }

  SECTION("sequence") {
    hyperloglog_array h(1, 12);
    for (int i : irange(1000000)) {
      h[0].insert(i);
    }
    REQUIRE(h[0].count() > 900000);
    REQUIRE(h[0].count() < 1100000);
  }

  SECTION("random") {
    hyperloglog_array h(1, 12);
    for (int i[[maybe_unused]] : irange(1000000)) {
      h[0].insert(bgl_random());
    }
    REQUIRE(h[0].count() > 900000);
    REQUIRE(h[0].count() < 1100000);
  }

  SECTION("merge") {
    hyperloglog_array h(3, 12);
    for (int i : irange(10000)) {
      h[0].insert(i);
      h[1].insert(2 * i);
      h[2].insert(i + 10000);
    }
    h[0].merge(h[1]);
    REQUIRE(h[0].count() > 13500);
    REQUIRE(h[0].count() < 16500);
    h[0].merge(h[2]);
    REQUIRE(h[0].count() > 18000);
    REQUIRE(h[0].count() < 22000);
  }
}
