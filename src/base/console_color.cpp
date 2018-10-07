#include "console_color.hpp"
#include "fmt.hpp"
#include <iostream>

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
//! defined when platform is Windows
#  define BGL_PLATFORM_WINDOWS
#  include <windows.h>
#endif

namespace bgl {
/**
 * @brief set foreground color of console.
 * @param os output stream. if |os| is neither of std::cout or std::cerr, do nothing
 * @param color color to set
 */
void set_console_color(std::ostream &os, console_color color) {
  if (os.rdbuf() != std::cout.rdbuf() && os.rdbuf() != std::cerr.rdbuf()) {
    return;
  }

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
  HANDLE handle = GetStdHandle(os.rdbuf() == std::cout.rdbuf() ? STD_OUTPUT_HANDLE
                                                               : STD_ERROR_HANDLE);
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
  fmt::print(os, "\x1b{}", esc);
#endif
}
} // namespace bgl
