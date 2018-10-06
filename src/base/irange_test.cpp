#include "extlib/catch.hpp"
#include "irange.hpp"
using namespace bgl;

TEST_CASE("irange", "[base]") {
  int sum = 0;
  for (int i : irange(5, 10)) {
    sum += i;
  }
  REQUIRE(sum == (5 + 6 + 7 + 8 + 9));
}
