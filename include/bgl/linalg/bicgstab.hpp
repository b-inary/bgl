#pragma once
#include "base.hpp"
#include "lu.hpp"
#include <cmath>
#include <utility>
#include <vector>

namespace bgl {
inline real_vector bicgstab(const sparse_matrix &A, const real_vector &b,
                            const std::pair<sparse_matrix, sparse_matrix> &precond_lu = {},
                            const real_vector &initial_guess = {},
                            double tol = 1e-8, int max_iter = 3000) {
  const std::size_t n = b.size();
  const double b_norm = b.norm();
  const bool use_precond = !precond_lu.first.empty();

  real_vector x(n);
  if (initial_guess.size() == n) {
    x = initial_guess;
  }

  real_vector r = b - A * x;
  real_vector rt(r);
  real_vector p(n);
  real_vector v(n);
  real_vector y(n);
  real_vector h(n);
  real_vector z(n);
  real_vector t(n);

  double rho = 1.0;
  double alpha = 1.0;
  double omega = 1.0;
  double beta, rtv, tt;

  for (int iter[[maybe_unused]] : irange(max_iter)) {
    double rho1 = rho;
    rho = inner_product(r, rt);
    if (rho == 0.0 || std::isinf(rho) || std::isnan(rho)) {
      CONSOLE_LOG("BiCGSTAB failed: errornous rho: {}", rho);
      goto error;
    }

    beta = (rho / rho1) * (alpha / omega);
    if (beta == 0.0 || std::isinf(beta) || std::isnan(beta)) {
      CONSOLE_LOG("BiCGSTAB failed: errornous beta: {}", beta);
      goto error;
    }

    p = r + beta * (p - omega * v);

    y = p;
    if (use_precond) {
      solve_lu(precond_lu, y);
      // for (int i : irange(n)) {
      //   if (std::isinf(y[i]) || std::isnan(y[i])) {
      //     CONSOLE_LOG("BiCGSTAB failed: errornous y[{}]: {}", i, y[i]);
      //     goto error;
      //   }
      // }
    }

    v = A * y;

    rtv = inner_product(rt, v);
    if (rtv == 0.0 || std::isinf(rtv) || std::isnan(rtv)) {
      CONSOLE_LOG("BiCGSTAB failed: errornous rtv: {}", rtv);
      goto error;
    }

    alpha = rho / rtv;
    if (std::isinf(alpha) || std::isnan(alpha)) {
      CONSOLE_LOG("BiCGSTAB failed: errornous alpha: {}", alpha);
      goto error;
    }

    x += alpha * y;
    r -= alpha * v;

    if (r.norm() < tol * b_norm) {
#ifdef SHOW_BICGSTAB_LOG
      real_vector residual = b - A * x;
      CONSOLE_LOG("BiCGSTAB converged\n  iter = {}\n  residual = {}",
                  iter + 0.5, residual.norm() / b_norm);
#endif
      return x;
    }

    z = r;
    if (use_precond) {
      solve_lu(precond_lu, z);
      // for (int i : irange(n)) {
      //   if (std::isinf(z[i]) || std::isnan(z[i])) {
      //     CONSOLE_LOG("BiCGSTAB failed: errornous z[{}]: {}", i, z[i]);
      //     goto error;
      //   }
      // }
    }

    t = A * z;
    tt = inner_product(t, t);
    if (tt == 0.0 || std::isinf(tt) || std::isnan(tt)) {
      CONSOLE_LOG("BiCGSTAB failed: errornous tt: {}", tt);
      goto error;
    }

    omega = inner_product(t, r) / tt;
    if (std::isinf(omega) || std::isnan(omega)) {
      CONSOLE_LOG("BiCGSTAB failed: errornous omega: {}", omega);
      goto error;
    }

    x += omega * z;
    r -= omega * t;

    if (r.norm() < tol * b_norm) {
#ifdef SHOW_BICGSTAB_LOG
      real_vector residual = b - A * x;
      CONSOLE_LOG("BiCGSTAB converged\n  iter = {}\n  residual = {}",
                  iter + 1, residual.norm() / b_norm);
#endif
      return x;
    }

    continue;

error:
    CONSOLE_LOG("iter = {}, residual = {}", iter + 1, r.norm() / b_norm);
    return x;
  }

#ifdef SHOW_BICGSTAB_LOG
  real_vector residual = b - A * x;
  CONSOLE_LOG("BiCGSTAB did not converge\n  max_iter = {}\n  residual = {}",
              max_iter, residual.norm() / b_norm);
#endif

  return x;
}
}  // namespace bgl
