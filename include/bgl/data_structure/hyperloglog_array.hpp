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
      insert_hash(internal_hash(elem ^ outer_.seed_));
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
    /// @see http://oertl.github.io/hyperloglog-sketch-estimation-paper/
    double count() const {
      int zero_count = 0;

      // treat as a 'fixed-point' real number: (1ull << (63 - outer_.log2k_)) == 1.0
      std::uint64_t sum = 0;

#ifdef __AVX2__
      const __m256i zeros = _mm256_setzero_si256();
      const __m256i ones_i64 = _mm256_set1_epi64x(1);
      const __m256i i8mask_i64 = _mm256_set1_epi64x(255);
      const __m256i shift_base_i8 = _mm256_set1_epi8(63 - outer_.log2k_);

      __m256i zero_count_i16[2] = { zeros, zeros };
      __m256i sum_i64 = zeros;

      for (int i = 0; i < outer_.k_; i += 32) {
        __m256i regs_i8 = _mm256_load_si256(reinterpret_cast<const __m256i*>(regs_ + i));

        __m256i tmp_i16;
        __m256i cmpzero_i8 = _mm256_sub_epi8(zeros, _mm256_cmpeq_epi8(regs_i8, zeros));
        tmp_i16 = _mm256_unpacklo_epi8(cmpzero_i8, zeros);
        zero_count_i16[0] = _mm256_add_epi16(zero_count_i16[0], tmp_i16);
        tmp_i16 = _mm256_unpackhi_epi8(cmpzero_i8, zeros);
        zero_count_i16[1] = _mm256_add_epi16(zero_count_i16[1], tmp_i16);

        __m256i shift_i8 = _mm256_sub_epi8(shift_base_i8, regs_i8);
        for (int j [[maybe_unused]] : irange(8)) {
          __m256i add_i64 = ones_i64;
          __m256i shift_i64 = _mm256_and_si256(shift_i8, i8mask_i64);
          add_i64 = _mm256_sllv_epi64(add_i64, shift_i64);
          sum_i64 = _mm256_add_epi64(sum_i64, add_i64);
          shift_i8 = _mm256_srli_epi64(shift_i8, 8);
        }
      }

      __m256i zero_count_i32 = zeros;
      for (int i : irange(2)) {
        __m256i tmp_i32;
        tmp_i32 = _mm256_unpacklo_epi16(zero_count_i16[i], zeros);
        zero_count_i32 = _mm256_add_epi32(zero_count_i32, tmp_i32);
        tmp_i32 = _mm256_unpackhi_epi16(zero_count_i16[i], zeros);
        zero_count_i32 = _mm256_add_epi32(zero_count_i32, tmp_i32);
      }
      zero_count_i32 = _mm256_hadd_epi32(zero_count_i32, zero_count_i32);
      zero_count_i32 = _mm256_hadd_epi32(zero_count_i32, zero_count_i32);
      zero_count += _mm256_extract_epi32(zero_count_i32, 0);
      zero_count += _mm256_extract_epi32(zero_count_i32, 4);

      sum += _mm256_extract_epi64(sum_i64, 0);
      sum += _mm256_extract_epi64(sum_i64, 1);
      sum += _mm256_extract_epi64(sum_i64, 2);
      sum += _mm256_extract_epi64(sum_i64, 3);
#else
      for (int i : irange(outer_.k_)) {
        zero_count += regs_[i] == 0;
        sum += 1ull << (63 - outer_.log2k_ - regs_[i]);
      }
#endif

      sum -= zero_count * (1ull << (63 - outer_.log2k_));

      double z = sum * outer_.norm_term_ + outer_.sigma_table_[zero_count];
      return outer_.k_ * outer_.k_ / z;
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
  hyperloglog_array(std::size_t count, int log2k, std::uint64_t seed = 0)
    : size_{count}
    , k_{1 << log2k}
    , log2k_{log2k}
    , seed_{internal_hash(seed)}
    , ary_(count << log2k, 32)
  {
    ASSERT_MSG(5 <= log2k && log2k <= 20,
               "parameter 'log2k' must be in range from 5 to 20\n"
               "  specified parameter: log2k = {}", log2k_);

    std::memset(ary_.data(), 0, size_ << log2k_);

    static const double alpha_inf = 0.5 / std::log(2);

    switch (log2k_) {
      case 5:  alpha_ = 0.6971226; break;
      case 6:  alpha_ = 0.7092085; break;
      case 7:  alpha_ = 0.7152712; break;
      case 8:  alpha_ = 0.7183076; break;
      case 9:  alpha_ = 0.7198271; break;
      case 10: alpha_ = 0.7205872; break;
      default: alpha_ = alpha_inf / (1.0 + 1.0798634 / k_); break;
    }

    norm_term_ = std::pow(2, log2k - 63) / alpha_;

    sigma_table_.resize(k_ + 1);
    for (int i : irange(k_ + 1)) {
      sigma_table_[i] = 2.0 * k_ * sigma(static_cast<double>(i) / k_) / (alpha_ + alpha_inf);
    }
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
  std::uint64_t seed_;
  double alpha_;
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
