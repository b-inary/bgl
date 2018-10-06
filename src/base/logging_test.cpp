#include "extlib/catch.hpp"
#include "logging.hpp"
using namespace bgl;

void logging_test() {
  fn_timer_stderr;
}

TEST_CASE("logging", "[logging]") {
  timer_stderr("timer test", logging_test);
  stderr_log("logging test: %s, %d", "hoge", 42);
}
