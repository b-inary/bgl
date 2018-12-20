#pragma once
#include "aligned_array.hpp"
#include "bgl/util/all.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#ifdef __AVX2__
#  include <immintrin.h>
#endif

namespace bgl {
/// precomputes and stores the parameters of HyperLogLog
class hyperloglog_params {
public:
  int m;
  int log2m;
  double normalize_coefficient;
  std::vector<double> sigma;

  hyperloglog_params(int log2m)
      : m{1 << log2m},
        log2m{log2m},
        normalize_coefficient{std::pow(2, -63) / alpha(log2m)},
        sigma{generate_sigma_table(log2m)} {}

private:
  static double alpha(int log2m) {
    switch (log2m) {
      case 5:
        return 0.6971226;
      case 6:
        return 0.7092085;
      case 7:
        return 0.7152712;
      case 8:
        return 0.7183076;
      case 9:
        return 0.7198271;
      case 10:
        return 0.7205872;
      default:
        return alpha_inf() / (1.0 + 1.0798634 / (1 << log2m));
    }
  }

  static double alpha_inf() { return 0.5 / std::log(2); }

  static double sigma_raw(double x) {
    if (x == 1.0) return 1.0 / 0.0;  // infinity
    double y = 0.5;
    double z = x;
    double prev_z = 0.0;
    while (prev_z != z) {
      prev_z = z;
      x *= x;
      y *= 2.0;
      z += x * y;
    }
    return z;
  }

  static std::vector<double> generate_sigma_table(int log2m) {
    std::vector<double> result;
    double coef = 2.0 / (alpha(log2m) + alpha_inf());  // empirical coefficient
    for (int i : irange((1 << log2m) + 1)) {
      result.push_back(coef * sigma_raw(static_cast<double>(i) / (1 << log2m)));
    }
    return result;
  }
};


/// HyperLogLog data structure (key type: std::uint64)
/// @see "HyperLogLog: the analysis of a near-optimal cardinality estimation algorithm"
///      (P. Flajolet et al.). In DMTCS'07.
class hyperloglog {
public:
  hyperloglog(std::uint8_t *regs, const hyperloglog_params &params)
      : regs_{regs}, params_{params} {}

  hyperloglog(const hyperloglog &rhs) = default;
  hyperloglog(hyperloglog &&rhs) = default;

  /// copy assignment operator
  /// @note *this and rhs must have the same number of registers (no check)
  hyperloglog &operator=(const hyperloglog &rhs) {
    std::memcpy(regs_, rhs.regs_, params_.m);
    return *this;
  }

  /// move assignment operator (actually do copying)
  /// @note *this and rhs must have the same number of registers (no check)
  hyperloglog &operator=(hyperloglog &&rhs) { return *this = rhs; }

  /// insert 64-bit integer element
  void insert(std::uint64_t elem) {
    std::uint64_t hash = internal_hash(elem);
    std::size_t index = hash >> (64 - params_.log2m);  // first |log2m| bit
    std::uint8_t rank = __builtin_ffsll(hash | (1LL << (63 - params_.log2m)));  // [1, 63 - log2m]
    regs_[index] = std::max(regs_[index], rank);  // update register
  }

  /// merge HyperLogLog counters
  /// @note *this and rhs must have the same number of registers (no check)
  void merge(const hyperloglog &rhs) {
#ifdef __AVX2__
    for (int i = 0; i < params_.m; i += 32) {
      __m256i x = _mm256_load_si256(reinterpret_cast<const __m256i *>(regs_ + i));
      __m256i y = _mm256_load_si256(reinterpret_cast<const __m256i *>(rhs.regs_ + i));
      __m256i m = _mm256_max_epu8(x, y);
      _mm256_store_si256(reinterpret_cast<__m256i *>(regs_ + i), m);
    }
#else
    for (int i : irange(params_.m)) {
      regs_[i] = std::max(regs_[i], rhs.regs_[i]);
    }
#endif
  }

  /// estimate the number of distinct elements (modified version)
  /// @see http://oertl.github.io/hyperloglog-sketch-estimation-paper/
  double count() const {
    int zero_count = 0;

    // treat as a 'fixed-point' real number: (1ull << (63 - log2m)) represents 1.0
    std::uint64_t sum = 0;

#ifdef __AVX2__
    const __m256i zeros = _mm256_setzero_si256();
    const __m256i ones_i64 = _mm256_set1_epi64x(1);
    const __m256i i8mask_i64 = _mm256_set1_epi64x(255);
    const __m256i shift_base_i8 = _mm256_set1_epi8(63 - params_.log2m);

    __m256i zero_count_i16[2] = {zeros, zeros};
    __m256i sum_i64 = zeros;

    for (int i = 0; i < params_.m; i += 32) {
      __m256i regs_i8 = _mm256_load_si256(reinterpret_cast<const __m256i *>(regs_ + i));

      __m256i tmp_i16;
      __m256i cmpzero_i8 = _mm256_sub_epi8(zeros, _mm256_cmpeq_epi8(regs_i8, zeros));
      tmp_i16 = _mm256_unpacklo_epi8(cmpzero_i8, zeros);
      zero_count_i16[0] = _mm256_add_epi16(zero_count_i16[0], tmp_i16);
      tmp_i16 = _mm256_unpackhi_epi8(cmpzero_i8, zeros);
      zero_count_i16[1] = _mm256_add_epi16(zero_count_i16[1], tmp_i16);

      __m256i shift_i8 = _mm256_sub_epi8(shift_base_i8, regs_i8);
      for (int j[[maybe_unused]] : irange(8)) {
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
    for (int i : irange(params_.m)) {
      zero_count += regs_[i] == 0;
      sum += 1ull << (63 - params_.log2m - regs_[i]);
    }
#endif

    sum -= zero_count * (1ull << (63 - params_.log2m));

    return params_.m / (sum * params_.normalize_coefficient + params_.sigma[zero_count]);
  }

  bool operator==(const hyperloglog &rhs) const {
    return std::memcmp(regs_, rhs.regs_, params_.m) == 0;
  }

  bool operator!=(const hyperloglog &rhs) const { return !(*this == rhs); }

private:
  uint8_t *regs_;
  const hyperloglog_params &params_;

  static std::uint64_t internal_hash(std::uint64_t x) {
    x *= 11400714819323198549ull;  // nearest prime to golden ratio
    x += 12345678900987654321ull;

    // http://mostlymangling.blogspot.com/2018/07/on-mixing-functions-in-fast-splittable.html
    x ^= rotr(x, 49) ^ rotr(x, 24);
    x *= 0x9fb21c651e98df25ull;
    x ^= x >> 28;
    x *= 0x9fb21c651e98df25ull;
    x ^= x >> 28;
    return x;
  }

  static std::uint64_t rotr(std::uint64_t x, int k) { return (x >> k) | (x << (64 - k)); }
};


/// an array of HyperLogLog counters
class hyperloglog_array {
private:
public:
  hyperloglog_array(std::size_t count, int log2m) : buf_(count << log2m, 32), params_(log2m) {
    ASSERT_MSG(5 <= log2m && log2m <= 20,
               "parameter 'log2m' must be in range from 5 to 20\n  given: log2m = {}", log2m);
    std::memset(buf_.data(), 0, count << log2m);
  }

  hyperloglog operator[](std::size_t pos) {
    return {buf_.data() + (pos << params_.log2m), params_};
  }

  bool operator==(const hyperloglog_array &rhs) const {
    return params_.m == rhs.params_.m && buf_ == rhs.buf_;
  }

  bool operator!=(const hyperloglog_array &rhs) const { return !(*this == rhs); }

private:
  aligned_array<uint8_t> buf_;
  hyperloglog_params params_;
};
}  // namespace bgl
