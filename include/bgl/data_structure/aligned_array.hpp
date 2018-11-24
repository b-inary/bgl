#pragma once
#include <new>
#include <cstdlib>
#include <cstring>

namespace bgl {
/// dynamic contigious array whose starting address is aligned
template <typename T>
class aligned_array {
 public:
  using value_type = T;

  aligned_array(std::size_t n, std::size_t align)
    : n_{n}
    , align_{align}
    , data_{aligned_malloc(n * sizeof(T), align)}
  {}

  aligned_array(const aligned_array &a) { *this = a; }

  aligned_array(aligned_array &&a) { *this = std::move(a); }

  ~aligned_array() { aligned_free(data_); }

  aligned_array& operator=(const aligned_array &rhs) {
    if (data_ == rhs.data_) return *this;
    if (n_ != rhs.n_ || align_ < rhs.align_) {
      aligned_free(data_);
      data_ = aligned_malloc(rhs.n_ * sizeof(T), rhs.align_);
    }
    n_ = rhs.n_;
    align_ = rhs.align_;
    std::memcpy(data_, rhs.data_, n_ * sizeof(T));
    return *this;
  }

  aligned_array& operator=(aligned_array &&rhs) {
    if (data_ == rhs.data_) return *this;
    aligned_free(data_);
    n_ = rhs.n_;
    align_ = rhs.align_;
    data_ = rhs.data_;
    rhs.data_ = nullptr;
    return *this;
  }

  value_type &operator[](size_t pos) {
    return data_[pos];
  }

  const value_type &operator[](size_t pos) const {
    return data_[pos];
  }

  value_type *data() noexcept {
    return data_;
  }

  const value_type *data() const noexcept {
    return data_;
  }

  std::size_t size() const noexcept {
    return n_;
  }

  std::size_t align() const noexcept {
    return align_;
  }

  bool operator==(const aligned_array &rhs) const {
    return n_ == rhs.n_ && std::memcmp(data_, rhs.data_, n_ * sizeof(T)) == 0;
  }

  bool operator!=(const aligned_array &rhs) const {
    return !(*this == rhs);
  }

 private:
  std::size_t n_ = 0;
  std::size_t align_ = 0;
  value_type *data_ = nullptr;

  static value_type *aligned_malloc(std::size_t size, std::size_t align) {
#if defined(__MINGW32__)
    void *p = _aligned_malloc(size, align);
#else
    void *p = std::aligned_alloc(align, size);
#endif
    if (p == nullptr) throw std::bad_alloc();
    return static_cast<value_type*>(p);
  }

  static void aligned_free(void *p) {
#if defined(__MINGW32__)
    _aligned_free(p);
#else
    std::free(p);
#endif
  }
};
} // namespace bgl
