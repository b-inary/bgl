#pragma once
#include "base.hpp"
#include "lu.hpp"
#include <cmath>
#include <utility>
#include <vector>

namespace bgl {
inline real_vector gmres(const sparse_matrix &A, const real_vector &b,
                         const std::pair<sparse_matrix, sparse_matrix> &precond = {},
                         double tol = 1e-6, int max_iter = 100, int restart = 30) {
  std::size_t n = b.size();
  bool use_precond = !precond.first.empty();
  double abs_tol = tol * b.norm();

  restart = std::min(static_cast<std::size_t>(restart), n);
  max_iter = std::min(static_cast<std::size_t>(max_iter), n / restart + 1);

  real_vector x(n, 1.0 / n);  // initial guess

  for (int outer_iter[[maybe_unused]] : irange(max_iter)) {
    int inner_iter = 0;
    std::vector<real_vector> q(restart + 1);
    std::vector<std::vector<double>> h(restart + 1, std::vector<double>(restart));
    std::vector<double> c(restart), s(restart), y(restart + 1);

    q[0] = b - A * x;
    if (use_precond) solve_lu(precond, q[0]);
    y[0] = q[0].norm();
    q[0] /= y[0];

    for (int j : irange(restart)) {
      ++inner_iter;

      // Arnoldi
      q[j + 1] = A * q[j];
      if (use_precond) solve_lu(precond, q[j + 1]);
      for (int i : irange(j + 1)) {
        h[i][j] = inner_product(q[i], q[j + 1]);
        q[j + 1] -= h[i][j] * q[i];
      }
      h[j + 1][j] = q[j + 1].norm();
      q[j + 1] /= h[j + 1][j];

      // previous Givens rotations
      for (int i : irange(j)) {
        double tmp = c[i] * h[i][j] + s[i] * h[i + 1][j];
        h[i + 1][j] = -s[i] * h[i][j] + c[i] * h[i + 1][j];
        h[i][j] = tmp;
      }

      // next rotation
      double gamma = std::hypot(h[j][j], h[j + 1][j]);
      c[j] = h[j][j] / gamma;
      s[j] = h[j + 1][j] / gamma;

      // Givens rotation on H
      h[j][j] = gamma;
      h[j + 1][j] = 0.0;

      // Givens rotation on y
      y[j + 1] = -s[j] * y[j];
      y[j] *= c[j];

      // convergence check
      if (std::abs(y[j + 1]) < abs_tol) break;
    }

    // Solve Hx = y: backward substitution
    for (int k : irange(inner_iter)) {
      int i = inner_iter - k - 1;
      for (int j : irange(i + 1, inner_iter)) {
        y[i] -= h[i][j] * y[j];
      }
      y[i] /= h[i][i];
    }

    // construct solution
    for (int i : irange(inner_iter)) {
      x += y[i] * q[i];
    }

    if (std::abs(y[inner_iter]) < abs_tol) {
      // CONSOLE_LOG("{}: solution converged\n  outer_iter = {}, inner_iter = {}\n  residual = {}",
      //             __func__, outer_iter + 1, inner_iter, (A * x - b).norm() / b.norm());
      return x;
    }
  }

  // CONSOLE_LOG("{}: solution did not converge\n  residual: {}", __func__,
  //             (A * x - b).norm() / b.norm());

  return x;
}
}  // namespace bgl
