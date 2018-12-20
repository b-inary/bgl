#pragma once
#include <algorithm>
#include <functional>
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
 * @brief remove elements from container |c| by predicate |p|
 * @param c container
 * @param p predicate
 * @return reference of processed container. instance is the same as |c|
 */
template <typename Container, typename Pred>
Container &remove_elements_if(Container &c, Pred p) {
  c.erase(std::remove_if(c.begin(), c.end(), p), c.end());
  return c;
}

/**
 * @brief remove deplicated elements from container |c| (by using predicate |p|)
 * @param c input container
 * @param p predicate
 * @return reference of processed container. instance is the same as |c|
 */
template <typename Container, typename Pred>
Container &remove_duplicates(Container &c, Pred p) {
  std::sort(c.begin(), c.end());
  c.erase(std::unique(c.begin(), c.end(), p), c.end());
  return c;
}

template <typename Container>
Container &remove_duplicates(Container &c) {
  return remove_duplicates(c, std::equal_to<typename Container::value_type>());
}
}  // namespace bgl
