#pragma once
#include "extlib/apathy.hpp"
#include "lambda.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <regex>

namespace bgl {
//! a super-tiny C++17 filesystem::path like class
class path {
public:
  path(const char *str = "") : path_{str} {}
  path(const std::string &str) : path_{str} {}

  path& operator/=(const path &rhs) {
    path_ /= rhs.path_;
    return *this;
  }

  friend path operator/(const path &lhs, const path &rhs) {
    path result(lhs);
    return result /= rhs;
  }

  path& remove_filename() {
    if (!path_.trailing_slash()) {
      path_.directory().up();
    }
    return *this;
  }

  path& replace_filename(const path &replacement) {
    remove_filename();
    path_ /= replacement.path_;
    return *this;
  }

  path& replace_extension(const path &replacement = path()) {
    std::string st = path_.stem();
    std::string ext = replacement.string();
    if (!ext.empty() && ext[0] != '.') ext = "." + ext;
    remove_filename();
    path_ /= st + ext;
    return *this;
  }

  std::string string() const {
    return path_.string();
  }

  path parent_path() const {
    path p = *this;
    p.remove_filename().path_.trim();
    return p;
  }

  path filename() const {
    return path_.filename();
  }

  path stem() const {
    return path_.stem();
  }

  path extension() const {
    return path_.extension();
  }

  /* simple test functions */

  static bool exists(const path &p) {
    return p.path_.exists();
  }

  static bool is_file(const path &p) {
    return p.path_.is_file();
  }

  static bool is_directory(const path &p) {
    return p.path_.is_directory();
  }

  /* traverse */

  static std::vector<path> find(const path &dir, const std::string &wildcard = "") {
    return find(dir, std::regex(regex_of_wildcard(wildcard)));
  }

  static std::vector<path> find(const path &dir, const std::regex &re) {
    if (!is_directory(dir)) return {};
    std::vector<path> results;
    auto ls = apathy::Path::listdir(dir.path_);
    for (const auto &el : ls) {
      path p = el.string();
      if (std::regex_search(p.string(), re)) {
        results.push_back(p);
      }
    }
    return results;
  }

  static std::vector<path> find_recursive(const path &dir, const std::string &wildcard = "") {
    return find_recursive(dir, std::regex(regex_of_wildcard(wildcard)));
  }

  static std::vector<path> find_recursive(const path &dir, const std::regex &re) {
    if (!is_directory(dir)) return {};
    std::vector<path> results;
    auto ls = apathy::Path::listdir(dir.path_);
    for (const auto &el : ls) {
      path p = el.string();
      if (is_directory(p)) {
        auto ls_recursive = find_recursive(p, re);
        results.insert(results.end(), ls_recursive.begin(), ls_recursive.end());
      } else if (std::regex_search(p.string(), re)) {
        results.push_back(p);
      }
    }
    return results;
  }

  /* path operations */

  static path current_path() {
    return apathy::Path::cwd().string();
  }

  static path relative(const path &p, const path &base = current_path()) {
    return apathy::Path::relative(p.path_, base.path_).string();
  }

  /* equality operators */

  friend bool operator==(const path &lhs, const path &rhs) noexcept {
    return lhs.path_ == rhs.path_;
  }

  friend bool operator!=(const path &lhs, const path &rhs) noexcept {
    return !(lhs == rhs);
  }

private:
  apathy::Path path_;

  static std::string regex_of_wildcard(const std::string &wildcard) {
    std::string result = wildcard;
    auto replace = lambda(target, replacement) {
      std::string::size_type pos = 0;
      while ((pos = result.find(target, pos)) != std::string::npos) {
        result.replace(pos, 1, replacement);
        pos += std::size(replacement) - 1;
      }
    };

    replace('.', "\\.");
    replace('*', ".*");
    replace('+', "\\+");
    replace('?', ".");
    replace('{', "\\{");
    replace('}', "\\}");

    return result;
  }
};
} // namespace bgl

namespace std {
inline std::ostream& operator<<(std::ostream &os, const bgl::path &p) {
  return os << '"' << p.string() << '"';
}
} // namespace std
