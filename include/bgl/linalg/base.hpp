#pragma once
#include "bgl/data_structure/aligned_array.hpp"
#include "bgl/graph/basic_graph.hpp"
#include "bgl/util/all.hpp"
#include <algorithm>
#include <cstring>
#include <random>

/* write simple code - optimization is task for compilers */

namespace bgl {
/// real dense vector
class real_vector : public aligned_array<double> {
public:
  real_vector(std::size_t n = 0) : real_vector(n, double{}) {}
  real_vector(std::size_t n, double value) : aligned_array(n, 32) {
    if (value == 0.0) {
      std::memset(data_, 0, n_ * sizeof(double));
    } else {
      std::fill(begin(), end(), value);
    }
  }

  /// vector-vector addition
  real_vector &operator+=(const real_vector &other) {
    ASSERT_MSG(n_ == other.n_, "size does not match");
    for (std::size_t i : irange(n_)) {
      (*this)[i] += other[i];
    }
    return *this;
  }

  /// vector-scalar addition
  real_vector &operator+=(double value) {
    for (std::size_t i : irange(n_)) {
      (*this)[i] += value;
    }
    return *this;
  }

  /// vector-vector subtraction
  real_vector &operator-=(const real_vector &other) {
    ASSERT_MSG(n_ == other.n_, "size does not match");
    for (std::size_t i : irange(n_)) {
      (*this)[i] -= other[i];
    }
    return *this;
  }

  /// vector-scalar subtraction
  real_vector &operator-=(double value) {
    for (std::size_t i : irange(n_)) {
      (*this)[i] -= value;
    }
    return *this;
  }

  /// vector-scalar multiplication
  real_vector &operator*=(double value) {
    for (std::size_t i : irange(n_)) {
      (*this)[i] *= value;
    }
    return *this;
  }

  /// vector-scalar division
  real_vector &operator/=(double value) {
    for (std::size_t i : irange(n_)) {
      (*this)[i] /= value;
    }
    return *this;
  }

  /// inner product
  double inner_product(const real_vector &other) const {
    ASSERT_MSG(n_ == other.n_, "size does not match");
    double result = 0.0;
    for (std::size_t i : irange(n_)) {
      result += (*this)[i] * other[i];
    }
    return result;
  }

  /// compute 2-norm
  double norm() const { return std::sqrt(inner_product(*this)); }

  /// compute sum of elements (i.e., 1-norm)
  double sum() const {
    double result = 0.0;
    for (std::size_t i : irange(n_)) {
      result += (*this)[i];
    }
    return result;
  }
};

inline real_vector operator+(const real_vector &lhs, const real_vector &rhs) {
  real_vector res(lhs);
  return res += rhs;
}

inline real_vector operator+(const real_vector &vec, double value) {
  real_vector res(vec);
  return res += value;
}

inline real_vector operator+(double value, const real_vector &vec) { return vec + value; }

inline real_vector operator-(const real_vector &lhs, const real_vector &rhs) {
  real_vector res(lhs);
  return res -= rhs;
}

inline real_vector operator-(const real_vector &vec, double value) {
  real_vector res(vec);
  return res -= value;
}

inline real_vector operator*(const real_vector &vec, double value) {
  real_vector res(vec);
  return res *= value;
}

inline real_vector operator*(double value, const real_vector &vec) { return vec * value; }

inline real_vector operator/(const real_vector &vec, double value) {
  real_vector res(vec);
  return res /= value;
}

inline double inner_product(const real_vector &lhs, const real_vector &rhs) {
  return lhs.inner_product(rhs);
}

/// Generate random unit vector on R^n
template <typename Generator>
real_vector generate_random_unit_vector(std::size_t n, Generator &gen) {
  // 1. Generate a vector v = (v_0, ..., v_{n-1}).
  //      - where v_i are independent standard normal random numbers
  // 2. Normalize v to make length 1.
  // Why it works? => standard multivariate normal distribution is invariant
  //                  under orthogonal transformations (which proves symmetry)
  real_vector res(n);
  std::normal_distribution<> dist{};
  for (std::size_t i : irange(n)) {
    res[i] = dist(gen);
  }
  return res / res.norm();
}

inline real_vector generate_random_unit_vector(std::size_t n) {
  return generate_random_unit_vector(n, bgl_random);
}


/// define sparse matrix type as weighted graph: A_ij = weight of edge from |i| to |j|
using sparse_matrix = wgraph<double>;

inline real_vector operator*(const sparse_matrix &A, const real_vector &x) {
  real_vector y(x.size());
  A.for_each_node(fn(i) {
    for (auto [j, v] : A.edges(i)) {
      y[i] += v * x[j];
    }
  });
  return y;
}
}  // namespace bgl
