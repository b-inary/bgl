#pragma once
#include <memory>
#include <type_traits>
#include <cstddef>
#include <iostream>

namespace bgl {
template <class T>
struct avx_aligned_allocator {
  using value_type = T;
  static const size_t align_bytes = 32;

  constexpr avx_aligned_allocator() = default;

  template <class U>
  constexpr avx_aligned_allocator(const avx_aligned_allocator<U>&) noexcept {}

  [[nodiscard]] T *allocate(std::size_t n) {
    if (n > std::size_t(-1) / sizeof(T)) throw std::bad_alloc();
#if defined(_MSC_VER) || defined(__MINGW32__)
    void *p = _aligned_malloc(n * sizeof(T), align_bytes);
#else
    void *p = std::aligned_alloc(align_bytes, n * sizeof(T));
#endif
    if (p == nullptr) throw std::bad_alloc();
    return static_cast<T*>(p);
  }

  void deallocate(T *p, std::size_t) {
#if defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(p);
#else
    std::free(p);
#endif
  }
};

template <class T1, class T2>
bool operator==(const avx_aligned_allocator<T1>&, const avx_aligned_allocator<T2>&) {
  return true;
}

template <class T1, class T2>
bool operator!=(const avx_aligned_allocator<T1>&, const avx_aligned_allocator<T2>&) {
  return false;
}
} // namespace bgl
