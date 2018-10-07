#include "console_color.hpp"
#include "fmt.hpp"
#include <cstdio>
#include <unistd.h>

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
//! defined when platform is Windows
#  define BGL_PLATFORM_WINDOWS
#  include <windows.h>
#endif

namespace bgl {
/**
 * @brief set foreground color of console.
 * @param color color to set
 * @param set_stderr if true, change stderr. otherwise, change stdout
 */
void set_console_color(console_color color, bool set_stderr) {
  if (isatty(fileno(set_stderr ? stderr : stdout))) {
#ifdef BGL_PLATFORM_WINDOWS
    WORD attr = [&] {
      switch (color) {
        case error:
          return FOREGROUND_INTENSITY | FOREGROUND_RED;
        case warning:
          return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
        case info:
          return FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN;
        default:
          return FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
      }
    }();
    HANDLE handle = GetStdHandle(set_stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, attr);
#else
    const char *esc = [&] {
      switch (color) {
        case error:
          return "[1;31m";
        case warning:
          return "[1;33m";
        case info:
          return "[1;36m";
        default:
          return "[0;39m";
      }
    }();
    fmt::print(set_stderr ? stderr : stdout, "\x1b{}", esc);
#endif
  }
}
} // namespace bgl
