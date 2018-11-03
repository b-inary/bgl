#include "../extlib/catch.hpp"

TEST_CASE("test name", "[tag]") {
  int a = 42;
  REQUIRE(a == 42);
}
