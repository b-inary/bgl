#include "../extlib/catch.hpp"
#include "bgl/linalg/lu.hpp"
using namespace bgl;

TEST_CASE("LU decomposition", "[lu][linalg]") {
  sparse_matrix A(100);
  for (int i : irange(100)) {
    for (int j : irange(100)) {
      if (i == j || bgl_random() < 0.1 * rng_t::max()) {
        A.add_edge(i, {j, static_cast<double>(bgl_random()) / rng_t::max()});
      }
    }
  }

  real_vector b = generate_random_unit_vector(100);
  real_vector x = b;
  solve_lu(lu_decomposition(A), x);

  REQUIRE((A * x - b).norm() < 1e-9);
}
