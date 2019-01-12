#include "../extlib/catch.hpp"
#include "bgl/linalg/gmres.hpp"
#include "bgl/linalg/lu.hpp"
using namespace bgl;

TEST_CASE("linsolve", "[lu][gmres][linalg]") {
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
  auto LU = lu_decomposition(A);
  solve_lu(LU, x);

  REQUIRE((A * x - b).norm() < 1e-9);

  real_vector y = gmres(A, b, {}, 1e-6, 100, 1);
  REQUIRE((A * y - b).norm() / b.norm() < 1e-6);

  real_vector z = gmres(A, b, LU);
  REQUIRE((A * z - b).norm() / b.norm() < 1e-6);
}
