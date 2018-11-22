#include "../extlib/catch.hpp"
#include "bgl/util/avx_allocator.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;
using namespace bgl;

TEST_CASE("aligned-allocator", "[util]") {
  vector<char, avx_aligned_allocator<char>> v1(32);
  vector<char> w(8);
  vector<char, avx_aligned_allocator<char>> v2(32);
  REQUIRE((reinterpret_cast<size_t>(v1.data()) & 31) == 0);
  REQUIRE((reinterpret_cast<size_t>(v2.data()) & 31) == 0);
}
