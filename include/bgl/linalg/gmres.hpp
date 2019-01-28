#pragma once
#include "base.hpp"
#include "lu.hpp"
#include <cmath>
#include <utility>
#include <vector>

namespace bgl {
inline real_vector gmres(const sparse_matrix &A, const real_vector &b,
                         const std::pair<sparse_matrix, sparse_matrix> &precond_lu = {},
                         double tol = 1e-6, int restart = 20, int max_iter = 100) {
  std::size_t n = b.size();
  bool use_precond = !precond_lu.first.empty();

  real_vector lub = b;
  if (use_precond) solve_lu(precond_lu, lub);
  double lub_norm = lub.norm();
  lub.clear();

  restart = std::min(static_cast<std::size_t>(restart), n);
  max_iter = std::min(static_cast<std::size_t>(max_iter), n / restart + 1);

  real_vector x(n);

  for (int outer_iter[[maybe_unused]] : irange(max_iter)) {
    int inner_iter = 0;
    std::vector<real_vector> q(restart + 1);
    std::vector<std::vector<double>> h(restart + 1, std::vector<double>(restart));
    std::vector<double> c(restart), s(restart), y(restart + 1);

    q[0] = b - A * x;
    if (use_precond) solve_lu(precond_lu, q[0]);
    y[0] = q[0].norm();
    q[0] /= y[0];

    for (int j : irange(restart)) {
      ++inner_iter;

      // Arnoldi
      q[j + 1] = A * q[j];
      if (use_precond) solve_lu(precond_lu, q[j + 1]);
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
      if (std::abs(y[j + 1]) <= tol * lub_norm) break;
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

    if (std::abs(y[inner_iter]) <= tol * lub_norm) {
#ifdef SHOW_GMRES_LOG
      real_vector residual = b - A * x;
      if (use_precond) solve_lu(precond_lu, residual);
      CONSOLE_LOG("GMRES({}) converged\n  outer_iter = {}, inner_iter = {}\n  residual = {}",
                  restart, outer_iter + 1, inner_iter, residual.norm() / lub_norm);
#endif
      return x;
    }
  }

#ifdef SHOW_GMRES_LOG
  real_vector residual = b - A * x;
  if (use_precond) solve_lu(precond_lu, residual);
  CONSOLE_LOG("GMRES({}) did not converge\n  max_iter = {}\n  residual = {}", restart, max_iter,
              residual.norm() / lub_norm);
#endif

  return x;
}
}  // namespace bgl
