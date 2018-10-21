#include "extlib/catch.hpp"
#include "bgl/util/container_manipulation.hpp"
#include "bgl/util/lambda.hpp"
#include <vector>
using namespace std;
using namespace bgl;

TEST_CASE("container", "[util]") {
  SECTION("filter") {
    vector<int> v{1, 5, 2, 4, 3};
    vector<int> w = filter(v, [](int x) { return x <= 3; });
    REQUIRE(w.size() == 3ul);
    REQUIRE(w[1] == 2);
  }

  SECTION("remove_elements") {
    vector<int> v{1, 5, 2, 4, 2, 3};
    remove_elements(v, 2);
    REQUIRE(v.size() == 4ul);
    REQUIRE(v[2] == 4);
  }

  SECTION("remove_elements_if") {
    vector<int> v{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    remove_elements_if(v, fn(x) { return x <= 3; });
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 4);
  }

  SECTION("remove_duplicates") {
    vector<int> v{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    remove_duplicates(v);
    REQUIRE(v.size() == 7);
  }
}
