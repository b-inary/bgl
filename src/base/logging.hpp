#pragma once
#include "console_color.hpp"
#include "fmt.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <chrono>

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
 * @param ... format string (of fmt library)
 */
#  define console_log(...) \
    _bgl_console_log(std::cerr, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief logging wrapper macro specifying output stream
 * @param os output stream for logging
 * @param ... format string (of fmt library)
 */
#  define write_log(os, ...) \
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
std::string get_date_string();

/**
 * @brief output string obtained by get_date_string() to |os|
 * @param os output stream
 */
void put_date_string(std::ostream &os);

template <typename Body>
void _bgl_timer(std::ostream &os, std::string_view title, const char *file,
                int line, const char *func, Body body) {
  auto start = std::chrono::high_resolution_clock::now();
  body();
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::string footer = fmt::format("(in {}(), {}:{})", func, file, line);

  put_date_string(os);
  if (&os == &std::cout || &os == &std::cerr) {
    set_console_color(console_color::info, &os == &std::cerr);
  }
  fmt::print(os, "timer: ");
  if (&os == &std::cout || &os == &std::cerr) {
    set_console_color(console_color::original, &os == &std::cerr);
  }
  fmt::print(os, "{}: {}[ms]\n  {}\n", title, elapsed.count(), footer);
}

class _bgl_fn_timer {
public:
  _bgl_fn_timer(std::ostream &os, const char *file, int line, const char *func)
    : os_{os}
    , func_{func}
    , footer_{fmt::format("({}:{})", file, line)}
    , start_{std::chrono::high_resolution_clock::now()} {}
  ~_bgl_fn_timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);

    put_date_string(os_);
    if (&os_ == &std::cout || &os_ == &std::cerr) {
        set_console_color(console_color::info, &os_ == &std::cerr);
    }
    fmt::print(os_, "fn-timer: ");
    if (&os_ == &std::cout || &os_ == &std::cerr) {
        set_console_color(console_color::original, &os_ == &std::cerr);
    }
    fmt::print(os_, "{}(): {}[ms]\n  {}\n", func_, elapsed.count(), footer_);
  }
private:
  std::ostream &os_;
  const char *func_;
  const std::string footer_;
  const std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

template <typename... Args>
void _bgl_console_log(std::ostream &os, const char *file, int line, const char *func,
                      const Args &...args) {
  std::string body = std::apply([](const auto &...args) { return fmt::format(args...); },
                                std::make_tuple(args...));
  put_date_string(os);
  if (&os == &std::cout || &os == &std::cerr) {
    set_console_color(console_color::info, &os == &std::cerr);
  }
  fmt::print(os, "log: ");
  if (&os == &std::cout || &os == &std::cerr) {
    set_console_color(console_color::original, &os == &std::cerr);
  }
  fmt::print(os, "{}\n  (in {}(), {}:{})\n", body, func, file, line);
}
} // namespace bgl
