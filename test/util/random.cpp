#include "../extlib/catch.hpp"
#include "bgl/util/random.hpp"
#include <random>
#include <cmath>
using namespace std;
using namespace bgl;

TEST_CASE("random", "[util]") {
  const int kNumPoints = 1000000;

  int c = 0;
  for (int i = 0; i < kNumPoints; ++i) {
    uniform_real_distribution<double> urd(0.0, 1.0);
    const double x = urd(bgl_random);
    const double y = urd(bgl_random);
    if (x * x + y * y < 1.0) ++c;
  }

  const double pi = c * 4.0 / kNumPoints;
  REQUIRE(3.13 < pi);
  REQUIRE(pi < 3.15);
}
