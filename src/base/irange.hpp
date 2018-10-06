#pragma once
#include <type_traits>

namespace bgl {
namespace {
template <typename T>
class irange_iterator {
public:
  irange_iterator(T x) : x_{x} {}
  const T& operator*() const { return x_; }
  // remark: this inequality operator does not satisfy commutativity
  bool operator!=(const irange_iterator &rhs) const { return x_ < rhs.x_; }
  void operator++() { ++x_; }
private:
  T x_;
};
} // unnamed namespace

template <typename T>
class irange_type {
public:
  irange_type(T m, T n) : m_{m}, n_{n} {}
  irange_iterator<T> begin() { return m_; }
  irange_iterator<T> end() { return n_; }
private:
  T m_;
  const T n_;
};

/**
 * @brief generate integer range adapter of [0, n)
 * @param n number of elements
 * @return range adapter of [0, n)
 */
template <typename T>
irange_type<T> irange(T n) {
  static_assert(std::is_integral<T>::value);
  return {0, n};
}

/**
 * @brief generate integer range adapter of [m, n)
 * @param m start value
 * @param n end value
 * @return range adapter of [m, n)
 */
template <typename T1, typename T2>
irange_type<typename std::common_type<T1, T2>::type> irange(T1 m, T2 n) {
  static_assert(std::is_integral<typename std::common_type<T1, T2>::type>::value);
  return {m, n};
}
} // namespace bgl
