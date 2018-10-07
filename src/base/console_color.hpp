#pragma once
#include <iostream>

namespace bgl {
//! candidates of console color
enum console_color {
  original,
  error,
  warning,
  info
};

/**
 * @brief set foreground color of console.
 * @param os output stream. if |os| is neither of std::cout or std::cerr, do nothing
 * @param color color to set
 */
void set_console_color(std::ostream &os, console_color color);
} // namespace bgl
