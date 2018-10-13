#pragma once
#include "extlib/rang.hpp"
#include "fmt.hpp"
#include "lambda.hpp"
#include "logging.hpp"
#include <iostream>
#include <tuple>
#include <stdexcept>

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

#define _bgl_check(is_error, expr, ...) \
  do {  \
    if (!(expr)) { \
      std::string msg = std::apply([](const auto &...args) { return fmt::format(args...); }, \
                                   std::make_tuple(__VA_ARGS__)); \
      bgl::put_date_string(std::cerr, lambda() { \
        fmt::print(std::cerr, "{}:{}: ", _bgl_source_path(__FILE__), __LINE__); \
        std::cerr << rang::style::bold << (is_error ? rang::fg::red : rang::fg::yellow); \
        fmt::print(std::cerr, is_error ? "error: " : "warning: "); \
        std::cerr << rang::style::reset << rang::fg::reset << msg; \
        if (msg == "assertion failed") { \
          fmt::print(std::cerr, "\n  assertion: {}", #expr); \
        } \
      }); \
      if (is_error) { \
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
