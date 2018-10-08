#include "extlib/catch.hpp"
#include "lambda.hpp"

TEST_CASE("lambda", "[util]") {
  auto const42 = lambda() { return 42; };
  REQUIRE(const42() == 42);

  auto plus = lambda(a, b) { return a + b; };
  REQUIRE(plus(2, 3) == 5);

  auto dot3 = lambda(a, b, c, x, y, z) { return a * x + b * y + c * z; };
  REQUIRE(dot3(1, 2, 3, 4, 5, 6) == 32);
}
