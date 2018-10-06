#pragma once
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "fmt/time.h"
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <chrono>
#include <ctime>
#include <cmath>

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
 * @param ... format string
 */
#  define stderr_log(...) \
    _bgl_console_log(std::cerr, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief logging wrapper macro specifying output stream
 * @param os output stream for logging
 * @param ... format string
 */
#  define console_log(os, ...) \
    _bgl_console_log(os, __FILE__, __LINE__, __func__, __VA_ARGS__)

#else
#  define timer(title, ...)         __VA_ARGS__()
#  define timer_os(os, title, ...)  __VA_ARGS__()
#  define auto_timer                static_cast<void>(0)
#endif


namespace bgl {
/**
 * @brief generate string representing current date and time
 * @return generated string
 */
std::string get_date_string() {
  auto now = std::chrono::high_resolution_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  auto now_second = std::chrono::system_clock::from_time_t(time);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - now_second);
  std::tm tm = *std::localtime(&time);
  return fmt::format("[{:%y-%m-%d %H:%M:%S}.{:02g}]", tm, std::floor(now_ms.count() / 10.0));
}

// implementation
template <typename Body>
void _bgl_timer(std::ostream &os, std::string_view title, const char *file,
                int line, const char *func, Body body) {
  auto start = std::chrono::high_resolution_clock::now();
  body();
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::string footer = fmt::format("(in {}(), {}:{})", func, file, line);
  fmt::print(os, "{} [timer] {}: {}[ms]\n  {}\n", get_date_string(), title,
             elapsed.count(), footer);
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
    fmt::print(os_, "{} [fn-timer] {}(): {}[ms]\n  {}\n", get_date_string(), func_,
               elapsed.count(), footer_);
  }
private:
  std::ostream &os_;
  const std::string footer_;
  const std::string_view func_;
  const std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

template <typename... Args>
void _bgl_console_log(std::ostream &os, const char *file, int line, const char *func,
                      const Args &...args) {
  std::string header = fmt::format("{} [log]", get_date_string());
  std::string footer = fmt::format("(in {}(), {}:{})", func, file, line);
  std::string body = std::apply([](const auto &...args) { return fmt::format(args...); },
                                std::make_tuple(args...));
  fmt::print(os, "{} {}\n  {}\n", header, body, footer);
}
} // namespace bgl
