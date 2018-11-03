#include "../extlib/catch.hpp"
#include "bgl/util/lambda.hpp"

TEST_CASE("lambda", "[util]") {
  auto const42 = fn() { return 42; };
  REQUIRE(const42() == 42);

  auto plus = fn(a, b) { return a + b; };
  REQUIRE(plus(2, 3) == 5);

  auto dot3 = fn(a, b, c, x, y, z) { return a * x + b * y + c * z; };
  REQUIRE(dot3(1, 2, 3, 4, 5, 6) == 32);
}
