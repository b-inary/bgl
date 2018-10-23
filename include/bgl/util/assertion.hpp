#pragma once
#include "extlib/rang.hpp"
#include "fmt.hpp"
#include "lambda.hpp"
#include "logging.hpp"
#include <iostream>
#include <tuple>
#include <stdexcept>

#ifndef NDEBUG
/// custom assertion macro: abort when |expr| is false.
/// @param expr condition expression
#define ASSERT(expr) ASSERT_MSG((expr), "assertion failed")

/// custom assertion macro: abort and print error message when |expr| is false.
/// @param expr condition expression
/// @param ... format string of error message
#define ASSERT_MSG(expr, ...) _bgl_assert(true, (expr), __VA_ARGS__)

/// custom assertion macro: continue running even when |expr| is false.
/// @param expr condition expression
#define EXPECT(expr) EXPECT_MSG((expr), "assertion failed")

/// custom assertion macro with message: continue running even when |expr| is false.
/// @param expr condition expression
/// @param ... format string of warning message
#define EXPECT_MSG(expr, ...) _bgl_assert(false, (expr), __VA_ARGS__)

#define _bgl_assert(is_error, expr, ...) \
  do {  \
    if (!(expr)) { \
      std::string msg = std::apply([](const auto &...args) { return fmt::format(args...); }, \
                                   std::make_tuple(__VA_ARGS__)); \
      bgl::put_date_string(std::cerr, fn() { \
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
#define ASSERT(expr)          (expr)
#define ASSERT_MSG(expr, ...) (expr)
#define EXPECT(expr)          (expr)
#define EXPECT_MSG(expr, ...) (expr)
#endif
