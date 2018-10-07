#include "fmt.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>

namespace bgl {
/**
 * @brief generate string representing current date and time
 * @return generated string
 */
std::string get_date_string() {
  auto now = std::chrono::high_resolution_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  auto now_second = std::chrono::system_clock::from_time_t(time);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - now_second);
  std::tm tm = *std::localtime(&time);
  return fmt::format("[{:%y-%m-%d %H:%M:%S}.{:02g}]", tm, std::floor(now_ms.count() / 10.0));
}

/**
 * @brief output string obtained by get_date_string() to |os|
 * @param os output stream
 */
void put_date_string(std::ostream &os) {
  fmt::print(os, "{} ", get_date_string());
}
} // namespace bgl
