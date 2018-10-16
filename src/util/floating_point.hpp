#pragma once
#include <limits>
#include <type_traits>
#include <cmath>

namespace bgl {
template <typename T>
typename std::enable_if_t<!std::is_floating_point_v<T>, bool> is_eq(T x, T y) noexcept {
  return x == y;
}

template <typename T>
typename std::enable_if_t<std::is_floating_point_v<T>, bool> is_eq(T x, T y) noexcept {
  static const int kUlp = 100;
  if (std::isinf(x) || std::isinf(y)) return x == y;
  return std::abs(x - y) <= std::numeric_limits<T>::epsilon() * std::max(x, y) * kUlp;
}

template <typename T>
bool is_zero(T x) noexcept {
  return is_eq(x, T{});
}

template <typename T>
bool is_lt(T x, T y) noexcept {
  return is_eq(x, y) ? false : (x < y);
}

template <typename T>
bool is_le(T x, T y) noexcept {
  return !is_lt(y, x);
}

template <typename T>
bool is_gt(T x, T y) noexcept {
  return is_lt(y, x);
}

template <typename T>
bool is_ge(T x, T y) noexcept {
  return !is_lt(x, y);
}
}
