#pragma once
#include "file.hpp"
#include "fmt.hpp"
#include "lambda.hpp"
#include "../extlib/rang.hpp"
#include <chrono>
#include <cmath>
#include <ctime>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <tuple>

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define _BGL_OS_WIN
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#define _BGL_EVAL(f, v) f(v)
#define _BGL_TO_STRING(s) _BGL_EVAL(_BGL_TO_STRING_HELPER, s)
#define _BGL_TO_STRING_HELPER(s) #s

/// easy timer logging macro that measures entire block.
/// usage: declare CONSOLE_TIMER at the top of block
#define CONSOLE_TIMER _bgl_timer _bgl_timer_instance(std::cerr, __FILE__, __LINE__)

/// easy timer logging macro that measures entire block.
/// usage: declare TIMER(os) at the top of block
/// @param os output stream for logging
#define TIMER(os) _bgl_timer _bgl_timer_instance((os), __FILE__, __LINE__)

/// logging macro
/// @param ... format string (of fmt library)
#define CONSOLE_LOG(...) _bgl_console_log(std::cerr, __FILE__, __LINE__, __VA_ARGS__)

/// logging macro for specified output stream
/// @param os output stream for logging
/// @param ... format string (of fmt library)
#define WRITE_LOG(os, ...) _bgl_console_log((os), __FILE__, __LINE__, __VA_ARGS__)


namespace bgl {
/// format integer with commas
/// @param num integer number
template <typename T>
typename std::enable_if_t<std::is_integral_v<T>, std::string> commify(T num) {
  class comma_numpunct : public std::numpunct<char> {
  protected:
    virtual char do_thousands_sep() const { return ','; }
    virtual std::string do_grouping() const { return "\03"; }
  };
  std::stringstream sst;
  sst.imbue(std::locale(std::locale(), new comma_numpunct));
  sst << num;
  return sst.str();
}

/// print right-aligned string when output stream is terminal
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

  fmt::print(os, "\x1b[{}G{} \b", width - str.length() + 1, str);
}

/// generate string representing current date and time
inline std::string get_date_string() {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  auto now_second = std::chrono::system_clock::from_time_t(time);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - now_second);
  std::tm tm = *std::localtime(&time);
  return fmt::format("[{:%Y-%m-%d %H:%M:%S}.{:02g}]", tm, std::floor(now_ms.count() / 10.0));
}

/// output string obtained by |get_date_string()| to |os|
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

class _bgl_timer {
public:
  _bgl_timer(std::ostream &os, const char *file, int line)
      : os_{os}, file_{file}, line_{line}, start_{std::chrono::high_resolution_clock::now()} {}
  ~_bgl_timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);

    put_date_string(os_, fn() {
      fmt::print(os_, "{}:{}: ", _bgl_source_path(file_), line_);
      os_ << rang::style::bold << rang::fg::cyan;
      fmt::print(os_, "timer: ");
      os_ << rang::style::reset << rang::fg::reset;
      fmt::print(os_, "{}[s]", elapsed.count() / 1000.0);
    });
  }

private:
  std::ostream &os_;
  const char *file_;
  const int line_;
  const std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

template <typename... Args>
void _bgl_console_log(std::ostream &os, const char *file, int line, const Args &... args) {
  std::string body =
      std::apply(fn(... args) { return fmt::format(args...); }, std::make_tuple(args...));

  put_date_string(os, fn() {
    fmt::print(os, "{}:{}: ", _bgl_source_path(file), line);
    os << rang::style::bold << rang::fg::cyan;
    fmt::print(os, "log: ");
    os << rang::style::reset << rang::fg::reset;
    fmt::print(os, "{}", body);
  });
}
}  // namespace bgl
