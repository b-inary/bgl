#pragma once
#include "bgl/util/all.hpp"
#include <vector>
#include <numeric>
#include <type_traits>

namespace bgl {
/// disjoint-set data structure
template <typename T, typename = std::enable_if<std::is_integral_v<T>>>
class union_find {
public:
  using integral_t = T;

  /// class initialization: O(n)
  union_find(integral_t count) : count_{count}, parent_(count), rank_(count) {
    std::iota(parent_.begin(), parent_.end(), 0u);
  }

  /// merge sets that includes x and includes y: approximately O(1)
  void unite(integral_t x, integral_t y) {
    x = find(x);
    y = find(y);
    if (x == y) return;
    if (rank_[x] < rank_[y]) {
      parent_[x] = y;
    } else {
      parent_[y] = x;
      if (rank_[x] == rank_[y]) ++rank_[x];
    }
    --count_;
  }

  /// determine if x and y are included by the same set: approximately O(1)
  bool is_same(integral_t x, integral_t y) {
    return find(x) == find(y);
  }

  /// return the number of disjoint sets: O(1)
  integral_t disjoint_count() {
    return count_;
  }

  /// return list of component IDs
  std::vector<integral_t> components() {
    for (auto i : irange(parent_.size())) find(i);
    return parent_;
  }

private:
  integral_t count_;
  std::vector<integral_t> parent_;
  std::vector<integral_t> rank_;

  /// find representative ID
  integral_t find(integral_t x) {
    return parent_[x] == x ? x : (parent_[x] = find(parent_[x]));
  }
};
} // namespace bgl
