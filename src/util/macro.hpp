#pragma once
#include "fmt.hpp"
#include "console_color.hpp"
#include "logging.hpp"
#include <iostream>
#include <tuple>
#include <cstdlib>

#ifndef NDEBUG
/**
 * @brief custom assertion macro. abort when |expr| is false
 * @param expr condition expression
 */
#define require(expr) \
  _bgl_check(true, expr, "assertion failed")

/**
 * @brief custom assertion macro with message. abort when |expr| is false
 * @param expr condition expression
 * @param ... message of format string
 */
#define require_msg(expr, ...) \
  _bgl_check(true, expr, __VA_ARGS__)

/**
 * @brief custom assertion macro. continue running even when |expr| is false
 * @param expr condition expression
 */
#define check(expr) \
  _bgl_check(false, expr, "assertion failed")

/**
 * @brief custom assertion macro with message. continue running even when |expr| is false
 * @param expr condition expression
 * @param ... message of format string
 */
#define check_msg(expr, ...) \
  _bgl_check(false, expr, __VA_ARGS__)

#define _bgl_check(abort, expr, ...) \
  do {  \
    if (!(expr)) { \
      std::string msg = std::apply([](const auto &...args) { return fmt::format(args...); }, \
                                   std::make_tuple(__VA_ARGS__)); \
      put_date_string(std::cerr); \
      set_console_color(std::cerr, abort ? console_color::error : console_color::warning); \
      fmt::print(std::cerr, abort ? "error: " : "warning: "); \
      set_console_color(std::cerr, console_color::original); \
      fmt::print(stderr, "{}\n  assertion: {}\n", msg, #expr); \
      fmt::print(stderr, "  (in {}(), {}:{})\n", __func__, __FILE__, __LINE__); \
      if (abort) { \
        std::exit(EXIT_FAILURE); \
      } \
    } \
  } while (false)
#else
#define require(expr)           (expr)
#define require_msg(expr, ...)  (expr)
#define check(expr)             (expr)
#define check_msg(expr, ...)    (expr)
#endif
