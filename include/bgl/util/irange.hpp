#pragma once
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace bgl {
template <typename T>
class irange_iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T *;
  using reference = T &;
  using iterator_category = std::forward_iterator_tag;
  constexpr irange_iterator(T value) : value_{value} {}
  constexpr T operator*() const { return value_; }
  // remark: this inequality operator does not satisfy commutativity
  constexpr bool operator!=(const irange_iterator &rhs) const { return value_ < rhs.value_; }
  void operator++() { ++value_; }

private:
  T value_;
};

template <typename T>
class irange_type {
public:
  constexpr irange_type(T start, T stop) : start_{start}, stop_{stop} {}
  constexpr irange_iterator<T> begin() { return start_; }
  constexpr irange_iterator<T> end() { return stop_; }

private:
  const T start_, stop_;
};

/**
 * @brief generate integer range adapter of [0, n)
 * @param n number of elements
 * @return range adapter of [0, n)
 */
template <typename T>
constexpr irange_type<T> irange(T n) {
  static_assert(std::is_integral_v<T>);
  return {0, n};
}

/**
 * @brief generate integer range adapter of [m, n)
 * @param m start value
 * @param n end value
 * @return range adapter of [m, n)
 */
template <typename T1, typename T2, typename CommonType = typename std::common_type<T1, T2>::type>
constexpr irange_type<CommonType> irange(T1 m, T2 n) {
  static_assert(std::is_integral_v<CommonType>);
  return {m, n};
}
}  // namespace bgl
