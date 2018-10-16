#pragma once
#include <typeinfo>
#include <string>
#ifdef __GNUG__
#  include <memory>
#  include <cstdlib>  // std::free
#  include <cxxabi.h>
#endif

// https://cpplover.blogspot.com/2014/10/ctypeidtypeinfotypeindex.html

namespace bgl {
#ifdef __GNUG__
struct _bgl_deleter {
  void operator()(char *p) const noexcept { std::free(p); }
};

//! demangle typeid. usage: demangle(typeid(arg))
inline std::string demangle(const std::type_info &ti) {
  int status = 0;
  std::unique_ptr<char, _bgl_deleter> p(abi::__cxa_demangle(ti.name(), nullptr, nullptr, &status));
  if (!p) return "demangle error";
  return p.get();
}
#else
inline std::string demangle(const std::type_info &ti) {
  return ti.name();
}
#endif

//! get typename string of |arg|
template <typename T>
std::string typename_of(const T &arg) {
  return demangle(typeid(arg));
}
} // namespace bgl
