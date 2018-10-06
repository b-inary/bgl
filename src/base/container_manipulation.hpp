#pragma once
#include <algorithm>
#include <iterator>

namespace bgl {
/**
 * @brief filter elements of |c| by predicate |p|
 * @param c container
 * @param p predicate
 * @return filtered container
 */
template <typename Container, typename Pred>
Container filter(const Container &c, Pred p) {
  Container res;
  std::copy_if(c.begin(), c.end(), std::back_inserter(res), p);
  return res;
}

/**
 * @brief remove elements equal to |value| from container |c|
 * @param c container
 * @param value removed value
 * @return reference of processed container. instance is the same as |c|
 */
template <typename Container, typename T>
Container &remove_elements(Container &c, const T &value) {
  c.erase(std::remove(c.begin(), c.end(), value), c.end());
  return c;
}

/**
 * @brief remove deplicated elements from container |c|
 * @param c input container
 * @return reference of processed container. instance is the same as |c|
 */
template <typename Container>
Container &remove_duplicates(Container &c)  {
  std::sort(c.begin(), c.end());
  c.erase(std::unique(c.begin(), c.end()), c.end());
  return c;
}
} // namespace bgl
