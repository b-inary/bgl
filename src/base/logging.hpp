#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <string_view>
#include <chrono>
#include <ctime>
#include <cstdarg>

#ifndef NDEBUG
/**
 * @brief easy timer logging macro. output to stderr
 * @param title title for logging
 * @param ... function that we want to measure (typically lambda function)
 * @note uses __VA_ARGS__ because comma can appear inside lambda function
 */
#  define timer_stderr(title, ...) \
    _bgl_timer(std::cerr, title, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief easy timer logging macro. specify output stream
 * @param os output stream for logging
 * @param title title for logging
 * @param ... function that we want to measure (typically lambda function)
 * @note uses __VA_ARGS__ because comma can appear inside lambda function
 */
#  define timer(os, title, ...) \
    _bgl_timer(os, title, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief easy timer logging macro that measures entire function.
 *        usage: declare fn_timer_stderr at the top of function
 */
#  define fn_timer_stderr \
    _bgl_fn_timer _bgl_fn_timer_instance(std::cerr, __FILE__, __LINE__, __func__)

/**
 * @brief easy timer logging macro that measures entire function.
 *        usage: declare fn_timer(os) at the top of function
 * @param os output stream for logging
 */
#  define fn_timer(os) \
    _bgl_fn_timer _bgl_fn_timer_instance(os, __FILE__, __LINE__, __func__)

/**
 * @brief logging wrapper macro that outputs to stderr
 * @param ... C-style format string (like printf())
 */
#  define stderr_log(...) \
    _bgl_console_log(std::cerr, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief logging wrapper macro specifying output stream
 * @param os output stream for logging
 * @param ... C-style format string (like printf())
 */
#  define console_log(os, ...) \
    _bgl_console_log(os, __FILE__, __LINE__, __func__, __VA_ARGS__)

#else
#  define timer(title, ...)         __VA_ARGS__()
#  define timer_os(os, title, ...)  __VA_ARGS__()
#  define auto_timer                static_cast<void>(0)
#endif


namespace bgl {
std::string get_date_string() {
  auto now = std::chrono::high_resolution_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&time);
  std::stringstream sst;
  sst << std::put_time(&tm, "%m/%d %H:%M:%S");
  return sst.str();
}

// implementation
template <typename Body>
void _bgl_timer(std::ostream &os, std::string_view title, const char *file,
                int line, const char *func, Body body) {
  auto start = std::chrono::high_resolution_clock::now();
  body();
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::string footer = std::string("(in ") + func + "(), " + file + ":" + std::to_string(line) + ")";
  os << "[timer] " << title << ": " << elapsed.count() << "[ms] " << footer << "\n";
}

class _bgl_fn_timer {
public:
  _bgl_fn_timer(std::ostream &os, const char *file, int line, const char *func)
    : os_{os}
    , footer_{std::string("(") + file + ":" + std::to_string(line) + ")"}
    , func_{func}
    , start_{std::chrono::high_resolution_clock::now()} {}
  ~_bgl_fn_timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    os_ << "[fn-timer] " << func_ << "(): " << elapsed.count() << "[ms] " << footer_ << '\n';
  }
private:
  std::ostream &os_;
  const std::string footer_;
  const std::string_view func_;
  const std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

void _bgl_console_log(std::ostream &os, const char *file, int line, const char *func,
                      const char *fmt, ...) {
  std::string header = std::string("[log] [") + get_date_string() + "] ";
  std::string footer = std::string(" (in ") + func + "(), " + file + ":" + std::to_string(line) + ")";

  va_list ap, ap2;
  va_start(ap, fmt);
  va_copy(ap2, ap);
  std::vector<char> buf(std::vsnprintf(nullptr, 0, fmt, ap) + 1);
  std::vsnprintf(buf.data(), buf.size(), fmt, ap2);
  va_end(ap);
  va_end(ap2);

  os << header << buf.data() << footer << '\n';
}
} // namespace bgl
