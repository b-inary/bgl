#include "extlib/catch.hpp"
#include "floating_point.hpp"
using namespace bgl;

TEST_CASE("floating point", "[util]") {
  REQUIRE(is_eq(3, 3) == true);
  REQUIRE(is_eq(3, 4) == false);
  REQUIRE(0.2 != (1.0 / std::sqrt(5) / std::sqrt(5)));
  REQUIRE(is_eq(0.2, 1.0 / std::sqrt(5) / std::sqrt(5)) == true);
  REQUIRE(is_eq(0.2, 0.2000001) == false);
}
