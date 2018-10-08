#include "extlib/catch.hpp"
#include "logging.hpp"
using namespace bgl;

void logging_test() {
  fn_timer_stderr;
}

TEST_CASE("logging", "[util]") {
  timer_stderr("timer test", logging_test);
  console_log("logging test: {}, {}", "hoge", 42);
  console_log_oneline("logging {}", "oneline");
}
