#include "extlib/catch.hpp"
#include "macro.hpp"
using namespace bgl;

TEST_CASE("macro", "[base]") {
  check(42 == 43);
  check_msg(42 == 43, "message {}", "desu");
}
