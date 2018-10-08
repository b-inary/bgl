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
  std::tm tm = *std::localtime(&time);
  return fmt::format("[{:%m/%d %H:%M:%S}]", tm);
}

/**
 * @brief output string obtained by get_date_string() to |os|
 * @param os output stream
 */
void put_date_string(std::ostream &os) {
  fmt::print(os, "{} ", get_date_string());
}
} // namespace bgl
