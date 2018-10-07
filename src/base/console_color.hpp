#pragma once

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
 * @param color color to set
 * @param set_stderr if true, change stderr. otherwise, change stdout
 */
void set_console_color(console_color color, bool set_stderr);
} // namespace bgl
