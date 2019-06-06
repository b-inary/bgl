#pragma once
#include "base.hpp"
#include "bgl/util/all.hpp"
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
  using edge_type = weighted_edge_t<double>;
  const node_t n = A.num_nodes();
  weighted_adjacency_list<double> L(n), U(n);
  std::vector<std::ptrdiff_t> ai_idx(n, -1);

  auto skip = [](auto begin_iter, auto end_iter, node_t value) {
    // ASSERT(begin_iter < end_iter && to(*begin_iter) < value);
    std::ptrdiff_t step = 1;
    auto last_iter = end_iter - 1;

    while (begin_iter < last_iter) {
      auto next_iter = std::min(begin_iter + step, last_iter);
      if (to(*next_iter) >= value) {
        end_iter = next_iter;
        break;
      }
      begin_iter = next_iter;
      step *= 16;
    }

    std::ptrdiff_t count = end_iter - begin_iter;
    while (count > 1) {
      step = count / 2;
      auto mid_iter = begin_iter + step;
      if (to(*mid_iter) < value) {
        begin_iter = mid_iter;
        count -= step;
      } else {
        count = step;
      }
    }

    return begin_iter + 1;
  };

  for (node_t i : A.nodes()) {
    const auto &es = A.edges(i);
    auto th_iter = es.begin();
    if (i < threshold) {
      th_iter = std::lower_bound(es.begin(), es.end(), threshold, compare_edge_node<edge_type>);
    }
    std::map<node_t, double> ai_exact(es.begin(), th_iter);
    std::vector<edge_type> ai(th_iter, es.end());
    const auto ai_end = ai.end();

    for (std::size_t k : irange(ai.size())) {
      ai_idx[to(ai[k])] = k;
    }

    for (auto [j, v] : ai_exact) {
      // exact LU
      auto uj_iter = U[j].begin();
      for (; uj_iter != U[j].end() && to(*uj_iter) < threshold; ++uj_iter) {
        ai_exact[to(*uj_iter)] -= v * weight(*uj_iter);
      }

      const auto uj_end = U[j].end();
      auto ai_iter = ai.begin();
      if (uj_iter == uj_end) continue;
      if (ai_iter == ai_end) continue;

      // incomplete LU
      if (uj_end - uj_iter < 4 * (ai_end - ai_iter)) {
        for (auto [k, w] : U[j]) {
          if (ai_idx[k] != -1) {
            ai[ai_idx[k]].second -= v * w;
          }
        }
      } else {
        while (true) {
          if (to(*uj_iter) == to(*ai_iter)) {
            ai_iter->second -= v * weight(*uj_iter);
            if (++uj_iter == uj_end) break;
            if (++ai_iter == ai_end) break;
          }
          if (to(*uj_iter) < to(*ai_iter)) {
            uj_iter = skip(uj_iter, uj_end, to(*ai_iter));
            if (uj_iter == uj_end) break;
          }
          if (to(*uj_iter) > to(*ai_iter)) {
            ai_iter = skip(ai_iter, ai_end, to(*uj_iter));
            if (ai_iter == ai_end) break;
          }
        }
      }
    }

    // incomplete LU
    auto ai_start = ai.begin();
    for (auto [j, v] : ai) {
      if (j >= i) break;

      const auto uj_end = U[j].end();
      auto uj_iter = U[j].begin();
      auto ai_iter = ++ai_start;

      if (uj_end - uj_iter < 4 * (ai_end - ai_iter)) {
        for (auto [k, w] : U[j]) {
          if (ai_idx[k] != -1) {
            ai[ai_idx[k]].second -= v * w;
          }
        }
      } else {
        while (true) {
          if (to(*uj_iter) == to(*ai_iter)) {
            ai_iter->second -= v * weight(*uj_iter);
            if (++uj_iter == uj_end) break;
            if (++ai_iter == ai_end) break;
          }
          if (to(*uj_iter) < to(*ai_iter)) {
            uj_iter = skip(uj_iter, uj_end, to(*ai_iter));
            if (uj_iter == uj_end) break;
          }
          if (to(*uj_iter) > to(*ai_iter)) {
            ai_iter = skip(ai_iter, ai_end, to(*uj_iter));
            if (ai_iter == ai_end) break;
          }
        }
      }
    }

    for (std::size_t k : irange(ai.size())) {
      ai_idx[to(ai[k])] = -1;
    }

    // construction & singular check
    double diag = 0.0;
    if (i < threshold) {
      diag = ai_exact[i];
      ASSERT_MSG(!is_zero(diag), "zero diagonal entry\n  row index = {}", i);
      auto it = ai_exact.upper_bound(i);
      L[i].assign(ai_exact.begin(), it);
      U[i].assign(it, ai_exact.end());
      U[i].insert(U[i].end(), ai.begin(), ai.end());
    } else {
      auto it = std::lower_bound(ai.begin(), ai.end(), i, compare_edge_node<edge_type>);
      if (to(*it) == i) diag = weight(*it);
      ASSERT_MSG(!is_zero(diag), "zero diagonal entry\n  row index = {}", i);
      L[i].assign(ai.begin(), ++it);
      U[i].assign(it, ai.end());
    }

    remove_elements_if(L[i], fn(e) { return weight(e) == 0.0; });
    remove_elements_if(U[i], fn(e) { return weight(e) == 0.0; });

    for (auto &e : U[i]) {
      e.second /= diag;
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
