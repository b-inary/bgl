#pragma once
#include "base.hpp"
#include "bgl/util/all.hpp"
#include <atomic>
#include <map>
#include <utility>
#include <vector>

namespace bgl {
/// LU decomposition
inline std::pair<sparse_matrix, sparse_matrix> lu_decomposition(const sparse_matrix &A) {
  const node_t n = A.num_nodes();
  weighted_adjacency_list<double> L(n), U(n);

  for (node_t i : A.nodes()) {
    auto &es = A.edges(i);
    std::map<node_t, double> ai(es.begin(), es.end());
    for (auto [j, v] : ai) {
      for (auto [k, w] : U[j]) {
        ai[k] -= v * w;
      }
    }

    double diag = ai[i];
    ASSERT_MSG(!is_zero(diag), "singular matrix\n  row index = {}\n  diagonal entry = {}", i, diag);

    for (auto [j, v] : ai) {
      if (is_zero(v)) continue;
      if (j <= i) {
        L[i].emplace_back(j, v);
      } else {
        U[i].emplace_back(j, v / diag);
      }
    }
  }

  return {std::move(L), std::move(U)};
}

/// incomplete LU decomposition
inline std::pair<sparse_matrix, sparse_matrix> ilu_decomposition(const sparse_matrix &A,
                                                                 node_t threshold = 0) {
  const node_t n = A.num_nodes();
  weighted_adjacency_list<double> L(n), U(n);

  for (node_t i : A.nodes()) {
    auto &es = A.edges(i);
    std::map<node_t, double> ai(es.begin(), es.end());
    for (auto [j, v] : ai) {
      for (auto [k, w] : U[j]) {
        if ((j < threshold && k < threshold) || ai.count(k)) {
          ai[k] -= v * w;
        }
      }
    }

    double diag = ai[i];
    ASSERT_MSG(!is_zero(diag), "singular matrix\n  row index = {}\n  diagonal entry = {}", i, diag);

    for (auto [j, v] : ai) {
      if (is_zero(v)) continue;
      if (j <= i) {
        L[i].emplace_back(j, v);
      } else {
        U[i].emplace_back(j, v / diag);
      }
    }
  }

  return {std::move(L), std::move(U)};
}

/// solve LUx = b; solution is stored to |b|
inline real_vector &solve_lu(const std::pair<sparse_matrix, sparse_matrix> &LU, real_vector &b) {
  std::size_t n = b.size();
  const sparse_matrix &L = LU.first;
  const sparse_matrix &U = LU.second;

  // forward substitution
  for (node_t i : L.nodes()) {
    for (node_t k : irange(L.outdegree(i) - 1)) {
      auto [j, v] = L.edge(i, k);
      b[i] -= b[j] * v;
    }
    b[i] /= weight(L.edges(i).back());
  }

  // backward substitution
  for (node_t i_rev : U.nodes()) {
    node_t i = n - i_rev - 1;
    for (auto [j, v] : U.edges(i)) {
      b[i] -= b[j] * v;
    }
  }

  return b;
}
}  // namespace bgl
