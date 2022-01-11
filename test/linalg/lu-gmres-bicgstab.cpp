#include "../extlib/catch.hpp"
#define SHOW_BICGSTAB_LOG
#define SHOW_GMRES_LOG
#include "bgl/linalg/bicgstab.hpp"
#include "bgl/linalg/gmres.hpp"
#include "bgl/linalg/lu.hpp"
using namespace bgl;

TEST_CASE("linsolve", "[lu][gmres][linalg]") {
  const int n = 30;
  sparse_matrix A(n);
  for (int i : irange(n)) {
    for (int j : irange(n)) {
      if (i == j || bgl_random() < 0.2 * static_cast<double>(rng_t::max())) {
        A.add_edge(i, {j, static_cast<double>(bgl_random()) / static_cast<double>(rng_t::max())});
      }
    }
  }

  const real_vector b = generate_random_unit_vector(n);
  auto LU = lu_decomposition(A);
  auto ILU = ilu_decomposition(A, n);
  auto ILU0 = ilu_decomposition(A, 0);

  real_vector x1 = b;
  solve_lu(LU, x1);
  REQUIRE((b - A * x1).norm() < 1e-12);

  real_vector x2 = b;
  solve_lu(ILU, x2);
  REQUIRE((b - A * x2).norm() < 1e-12);

  real_vector y1 = gmres(A, b, {}, {}, 1e-8, n, 1);
  REQUIRE((b - A * y1).norm() / b.norm() < 1e-8);

  real_vector y2 = bicgstab(A, b, {}, {}, 1e-8, 10 * n);
  REQUIRE((b - A * y2).norm() / b.norm() < 1e-8);

  real_vector z1 = gmres(A, b, ILU0, {}, 1e-8);
  real_vector lur = b - A * z1;
  real_vector lub = b;
  solve_lu(ILU0, lur);
  solve_lu(ILU0, lub);
  REQUIRE(lur.norm() / lub.norm() < 1e-8);

  real_vector z2 = bicgstab(A, b, ILU0, {}, 1e-8);
  REQUIRE((b - A * z2).norm() / b.norm() < 1e-8);
}
