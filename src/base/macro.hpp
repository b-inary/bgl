#pragma once
#include "fmt.hpp"
#include "console_color.hpp"
#include "logging.hpp"
#include <iostream>
#include <tuple>
#include <cstdlib>

//! macro for suppressing unused variable warning
#define unused(x) (static_cast<void>(x))

/**
 * @brief custom assertion macro. abort when |expr| is false
 * @param expr condition expression
 */
#define require(expr) \
  _bgl_check(true, expr, #expr, __FILE__, __LINE__, __func__, "")

/**
 * @brief custom assertion macro with message. abort when |expr| is false
 * @param expr condition expression
 * @param ... message of format string
 */
#define require_msg(expr, ...) \
  _bgl_check(true, expr, #expr, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief custom assertion macro. continue running even when |expr| is false
 * @param expr condition expression
 */
#define check(expr) \
  _bgl_check(false, expr, #expr, __FILE__, __LINE__, __func__, "")

/**
 * @brief custom assertion macro with message. continue running even when |expr| is false
 * @param expr condition expression
 * @param ... message of format string
 */
#define check_msg(expr, ...) \
  _bgl_check(false, expr, #expr, __FILE__, __LINE__, __func__, __VA_ARGS__)

namespace bgl {
template <typename Expr, typename... Args>
void _bgl_check(bool abort, Expr expr, const char* expr_str, const char *file, int line,
                const char *func, const Args &...args) {
  if (!(expr)) {
    std::string msg = std::apply([](const auto &...args) { return fmt::format(args...); },
                                 std::make_tuple(args...));
    if (!msg.empty()) {
      msg = ": " + msg;
    }

    put_date_string(std::cerr);
    set_console_color(abort ? console_color::error : console_color::warning, true);
    fmt::print(std::cerr, abort ? "error: " : "warning: ");
    set_console_color(console_color::original, true);
    fmt::print(stderr, "check failed{}\n  expression: {}\n", msg, expr_str);
    fmt::print(stderr, "  (in {}(), {}:{})\n", func, file, line);

    if (abort) {
      std::exit(EXIT_FAILURE);
    }
  }
}
} // namespace bgl
