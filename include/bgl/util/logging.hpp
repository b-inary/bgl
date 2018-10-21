#pragma once
#include "extlib/rang.hpp"
#include "fmt.hpp"
#include "file.hpp"
#include "lambda.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <functional>
#include <tuple>
#include <chrono>
#include <cmath>
#include <ctime>

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#  define _BGL_OS_WIN
#  include <windows.h>
#else
#  include <sys/ioctl.h>
#endif

#define _BGL_EVAL(f, v) f(v)
#define _BGL_TO_STRING(s) _BGL_EVAL(_BGL_TO_STRING_HELPER, s)
#define _BGL_TO_STRING_HELPER(s) #s

/**
 * @brief easy timer logging macro. output to stderr
 * @param title title for logging
 * @param ... function that we want to measure (typically lambda function)
 * @note uses __VA_ARGS__ because comma can appear inside lambda function
 */
#define console_timer(title, ...) \
  _bgl_timer(std::cerr, title, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief easy timer logging macro. specify output stream
 * @param os output stream for logging
 * @param title title for logging
 * @param ... function that we want to measure (typically lambda function)
 * @note uses __VA_ARGS__ because comma can appear inside lambda function
 */
#define timer(os, title, ...) \
  _bgl_timer(os, title, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief easy timer logging macro that measures entire function.
 *        usage: declare fn_timer_stderr at the top of function
 */
#define console_fn_timer \
  _bgl_fn_timer _bgl_fn_timer_instance(std::cerr, __FILE__, __LINE__, __func__)

/**
 * @brief easy timer logging macro that measures entire function.
 *        usage: declare fn_timer(os) at the top of function
 * @param os output stream for logging
 */
#define fn_timer(os) \
  _bgl_fn_timer _bgl_fn_timer_instance(os, __FILE__, __LINE__, __func__)

/**
 * @brief logging wrapper macro that outputs to stderr
 * @param ... format string (of fmt library)
 */
#define console_log(...) \
  _bgl_console_log(std::cerr, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief logging wrapper macro specifying output stream
 * @param os output stream for logging
 * @param ... format string (of fmt library)
 */
#define write_log(os, ...) \
  _bgl_console_log(os, __FILE__, __LINE__, __VA_ARGS__)


namespace bgl {
//! print right-aligned string when output stream is terminal
inline void pretty_append(std::ostream &os, std::function<void()> body, const std::string &str) {
  if (!rang::rang_implementation::isTerminal(os.rdbuf())) {
    os << str << ' ';
    body();
    os << '\n';
    return;
  }

  body();
  os << std::string(str.length(), ' ') << std::flush;

#ifdef _BGL_OS_WIN
  HANDLE handle = rang::rang_implementation::getConsoleHandle(os.rdbuf());
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(handle, &csbi);
  int width = csbi.dwSize.X;

  if (!rang::rang_implementation::supportsAnsi(os.rdbuf())) {
    COORD pos = csbi.dwCursorPosition;
    pos.X = width - str.length();
    SetConsoleCursorPosition(handle, pos);
    os << str;
    return;
  }
#else
  struct winsize ws;
  ioctl(0, TIOCGWINSZ, &ws);
  int width = ws.ws_col;
#endif

  fmt::print(os, "\x1b[{}G{}", width - str.length() + 1, str);
}

//! generate string representing current date and time
inline std::string get_date_string() {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  auto now_second = std::chrono::system_clock::from_time_t(time);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - now_second);
  std::tm tm = *std::localtime(&time);
  return fmt::format("[{:%Y-%m-%d %H:%M:%S}.{:02g}]", tm, std::floor(now_ms.count() / 10.0));
}

//! output string obtained by get_date_string() to |os|
inline void put_date_string(std::ostream &os, std::function<void()> body) {
  pretty_append(os, body, get_date_string());
}

inline std::string _bgl_source_path(const char *file) {
#ifdef _BGL_DIRECTORY
  path bgl_dir = _BGL_TO_STRING(_BGL_DIRECTORY);
  path src = bgl_dir / "build" / file;
  return path::relative(src).string();
#else
  return file;
#endif
}

template <typename Body>
void _bgl_timer(std::ostream &os, std::string_view title, const char *file, int line, Body body) {
  auto start = std::chrono::system_clock::now();
  body();
  auto end = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  put_date_string(os, fn() {
    fmt::print(os, "{}:{}: ", _bgl_source_path(file), line);
    os << rang::style::bold << rang::fg::cyan;
    fmt::print(os, "timer: ");
    os << rang::style::reset << rang::fg::reset;
    fmt::print(os, "{}: {}[ms]", title, elapsed.count());
  });
}

class _bgl_fn_timer {
public:
  _bgl_fn_timer(std::ostream &os, const char *file, int line, const char *func)
    : os_{os}
    , file_{file}
    , func_{func}
    , line_{line}
    , start_{std::chrono::high_resolution_clock::now()} {}
  ~_bgl_fn_timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);

    put_date_string(os_, fn() {
      fmt::print(os_, "{}:{}: ", _bgl_source_path(file_), line_);
      os_ << rang::style::bold << rang::fg::cyan;
      fmt::print(os_, "fn-timer: ");
      os_ << rang::style::reset << rang::fg::reset;
      fmt::print(os_, "{}(): {}[ms]", func_, elapsed.count());
    });
  }
private:
  std::ostream &os_;
  const char *file_;
  const char *func_;
  const int line_;
  const std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

template <typename... Args>
void _bgl_console_log(std::ostream &os, const char *file, int line, const Args &...args) {
  std::string body = std::apply([](const auto &...args) { return fmt::format(args...); },
                                std::make_tuple(args...));

  put_date_string(os, fn() {
    fmt::print(os, "{}:{}: ", _bgl_source_path(file), line);
    os << rang::style::bold << rang::fg::cyan;
    fmt::print(os, "log: ");
    os << rang::style::reset << rang::fg::reset;
    fmt::print(os, "{}", body);
  });
}
} // namespace bgl
