#include "extlib/catch.hpp"

TEST_CASE("test", "[test]") {
  int a = 42;
  REQUIRE(a == 42);
}
