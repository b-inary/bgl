#pragma once
#include "bgl/util/all.hpp"
#include "bgl/data_structure/aligned_array.hpp"
#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cstdint>

#ifdef __AVX2__
#  include <immintrin.h>
#endif

namespace bgl {
/// an array of HyperLogLog counters (key type: std::uint64)
/// @see "HyperLogLog: the analysis of a near-optimal cardinality estimation algorithm"
///      (P. Flajolet et al.). In AOFA'07.
class hyperloglog_array {
private:
  class hyperloglog_impl {
  public:
    hyperloglog_impl(hyperloglog_array &outer, std::uint8_t *regs)
      : outer_{outer}, regs_{regs} {}

    hyperloglog_impl(const hyperloglog_impl &rhs) = default;
    hyperloglog_impl(hyperloglog_impl &&rhs) = default;

    /// prohibit assignment operation
    /// (this class is a kind of reference, so assignment is confusing)
    hyperloglog_impl& operator=(const hyperloglog_impl &rhs) = delete;
    hyperloglog_impl& operator=(hyperloglog_impl &&rhs) = delete;

    /// insert 64-bit integer element
    void insert(std::uint64_t elem) {
      insert_hash(internal_hash(elem));
    }

    /// insert 64-bit hash value directly
    void insert_hash(std::uint64_t hash) {
      // index: first |log2k| bit
      std::size_t index = hash >> (64 - outer_.log2k_);
      // rank: position of the rightmost '1' (range: [1, 63 - log2k])
      std::uint8_t rank = __builtin_ffsll(hash | (1LL << (63 - outer_.log2k_)));
      // update register
      regs_[index] = std::max(regs_[index], rank);
    }

    /// merge HyperLogLog counters
    /// @note *this and rhs must have the same number of registers (no check)
    void merge(const hyperloglog_impl &rhs) {
#ifdef __AVX2__
      for (int i = 0; i < outer_.k_; i += 32) {
        __m256i x = _mm256_load_si256(reinterpret_cast<const __m256i*>(regs_ + i));
        __m256i y = _mm256_load_si256(reinterpret_cast<const __m256i*>(rhs.regs_ + i));
        __m256i m = _mm256_max_epu8(x, y);
        _mm256_store_si256(reinterpret_cast<__m256i*>(regs_ + i), m);
      }
#else
      for (int i : irange(outer_.k_)) {
        regs_[i] = std::max(regs_[i], rhs.regs_[i]);
      }
#endif
    }

    /// estimate the number of distinct elements (modified version)
    /// @see: http://oertl.github.io/hyperloglog-sketch-estimation-paper/
    double count() const {
      int zero_count = 0;

      // treat as a 'fixed-point' real number: (1ull << (63 - outer_.log2k_)) == 1.0
      std::uint64_t sum = 0;

      for (int i = 0; i < outer_.k_; ++i) {
        zero_count += regs_[i] == 0;
        sum += 1ull << (63 - outer_.log2k_ - regs_[i]);
      }

      sum -= zero_count * (1ull << (63 - outer_.log2k_));

      double z = sum * outer_.norm_term_ + outer_.sigma_table_[zero_count];
      static const double alpha = 0.5 / std::log(2);
      return alpha * outer_.k_ * outer_.k_ / z;
    }

    bool operator==(const hyperloglog_impl &rhs) const {
      return std::memcmp(regs_, rhs.regs_, outer_.k_) == 0;
    }

    bool operator!=(const hyperloglog_impl &rhs) const {
      return !(*this == rhs);
    }

   private:
    hyperloglog_array &outer_;
    uint8_t *regs_;
  };

 public:
  hyperloglog_array(std::size_t count, int log2k)
    : size_{count}
    , k_{1 << log2k}
    , log2k_{log2k}
    , norm_term_{std::pow(2, log2k - 63)}
    , ary_(count << log2k, 32)
  {
    ASSERT_MSG(5 <= log2k && log2k <= 20,
               "parameter 'log2k' must be in range from 5 to 20\n"
               "  specified parameter: log2k = {}", log2k_);

    std::memset(ary_.data(), 0, size_ << log2k_);

    sigma_table_.resize(k_ + 1);
    for (int i : irange(k_ + 1)) {
      sigma_table_[i] = k_ * sigma(static_cast<double>(i) / k_);
    }
  }

  void swap(hyperloglog_array &other) noexcept {
    hyperloglog_array tmp = std::move(other);
    other = std::move(*this);
    *this = std::move(tmp);
  }

  hyperloglog_impl operator[](std::size_t pos) {
    return {*this, ary_.data() + (pos << log2k_)};
  }

  bool operator==(const hyperloglog_array &rhs) const {
    return k_ == rhs.k_ && ary_ == rhs.ary_;
  }

  bool operator!=(const hyperloglog_array &rhs) const {
    return !(*this == rhs);
  }

 private:
  size_t size_;
  int k_;
  int log2k_;
  double norm_term_;
  aligned_array<std::uint8_t> ary_;
  std::vector<double> sigma_table_;

  static double sigma(double x) {
    if (x == 1.0) return 1.0 / 0.0; // infinity
    double y = 1.0;
    double z = x;
    double prev_z;
    do {
      prev_z = z;
      x = x * x;
      z += x * y;
      y *= 2;
    } while (z != prev_z);
    return z;
  }

  static std::uint64_t internal_hash(std::uint64_t x) {
    // multiply nearest prime to golden ratio
    x *= 0x9e3779b97f4a7c55ull;
    x += 1;

    // MurmurHash3's 64-bit finalizer
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdull;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ull;
    x ^= x >> 33;
    return x;
  }
};
} // namespace bgl
