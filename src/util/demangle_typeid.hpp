#pragma once
#include <typeinfo>
#include <string>
#include <memory>
#include <cstdlib>  // std::free
#include <cxxabi.h>

// https://cpplover.blogspot.com/2014/10/ctypeidtypeinfotypeindex.html

namespace bgl {
namespace {
struct deleter {
  void operator()(char *p) const noexcept { std::free(p); }
};
}

/**
 * @brief demangle typeid. usage: demangle(typeid(arg))
 * @param ti type_info class
 */
std::string demangle(const std::type_info &ti) {
  int status = 0;
  std::unique_ptr<char, deleter>
    p(abi::__cxa_demangle(ti.name(), nullptr, nullptr, &status));
  if (!p) return "demangle error";
  return p.get();
}
} // namespace bgl
