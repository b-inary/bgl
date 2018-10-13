#include "extlib/catch.hpp"
#include "file.hpp"
#include <iostream>
#include <algorithm>
using namespace bgl;

TEST_CASE("file", "[util]") {
  REQUIRE(path().string() == "");
  REQUIRE(path(".").string() == ".");
  REQUIRE(path("foo/bar").string() == "foo/bar");
  REQUIRE(path("foo/bar/").string() == "foo/bar/");
  REQUIRE(path("foo/bar/.").string() == "foo/bar/");
  REQUIRE(path("foo/./bar/..").string() == "foo/");
  REQUIRE(path("foo/.///bar/../").string() == "foo/");
  REQUIRE(path("../../").string() == "../..");
  REQUIRE(path("foo/..").string() == ".");

  REQUIRE((path("foo") / "bar").string() == "foo/bar");
  REQUIRE(("foo/" / path("bar")).string() == "foo/bar");
  REQUIRE(("foo/" / path("/bar")).string() == "/bar");
  REQUIRE((path("/foo") / path("/bar/")).string() == "/bar/");
  REQUIRE((path("/") / path("bar")).string() == "/bar");
  REQUIRE((path("foo/") / path("..")).string() == ".");
  REQUIRE((path("/") / path("..")).string() == "/");

  REQUIRE(path("foo/bar").remove_filename().string() == "foo/");
  REQUIRE(path("foo/").remove_filename().string() == "foo/");
  REQUIRE(path("/foo").remove_filename().string() == "/");
  REQUIRE(path("/").remove_filename().string() == "/");

  REQUIRE(path("foo/bar.txt").replace_filename("a.md").string() == "foo/a.md");
  REQUIRE(path("foo/").replace_filename("a.md").string() == "foo/a.md");
  REQUIRE(path("/").replace_filename("a.md").string() == "/a.md");

  REQUIRE(path("/foo.txt").replace_extension(".md").string() == "/foo.md");
  REQUIRE(path("/foo.txt").replace_extension("md").string() == "/foo.md");
  REQUIRE(path("/foo").replace_extension("md").string() == "/foo.md");

  REQUIRE(path("/foo/bar.txt").parent_path().string() == "/foo");
  REQUIRE(path("/foo/bar/").parent_path().string() == "/foo/bar");
  REQUIRE(path("/foo/bar/.").parent_path().string() == "/foo/bar");
  REQUIRE(path("/").parent_path().string() == "/");

  REQUIRE(path("foo").filename().string() == "foo");
  REQUIRE(path("/foo/bar.txt").filename().string() == "bar.txt");
  REQUIRE(path("/foo/bar/").filename().string() == "");
  REQUIRE(path("/").filename().string() == "");
  REQUIRE(path(".").filename().string() == ".");
  REQUIRE(path("..").filename().string() == "..");

  REQUIRE(path("foo").stem().string() == "foo");
  REQUIRE(path("/foo/.bar").stem().string() == ".bar");
  REQUIRE(path("/foo/bar.txt").stem().string() == "bar");
  REQUIRE(path("/foo/bar.tar.gz").stem().string() == "bar.tar");
  REQUIRE(path("/foo/").stem().string() == "");
  REQUIRE(path(".").stem().string() == ".");
  REQUIRE(path("..").stem().string() == "..");
  REQUIRE(path("..foo").stem().string() == ".");

  REQUIRE(path("foo").extension().string() == "");
  REQUIRE(path("foo/bar.txt").extension().string() == ".txt");
  REQUIRE(path("/foo/bar.tar.gz").extension().string() == ".gz");
  REQUIRE(path("foo/bar.").extension().string() == ".");
  REQUIRE(path("/foo/").extension().string() == "");
  REQUIRE(path("/foo/.").extension().string() == "");
  REQUIRE(path("..").extension().string() == "");
  REQUIRE(path(".hidden").extension().string() == "");
  REQUIRE(path("..foo").extension().string() == ".foo");

  REQUIRE(path::relative("foo/bar", "foo").string() == "bar");
  REQUIRE(path::relative("foo/bar", "foo/").string() == "bar");
  REQUIRE(path::relative("foo/bar/", "foo/").string() == "bar/");
  REQUIRE(path::relative("foo/bar", "baz").string() == "../foo/bar");
  // REQUIRE(path::relative("foo/bar", "../").string() == "bgl/foo/bar");

  auto ls = path::find(".");
  REQUIRE(std::count(ls.begin(), ls.end(), path("src")));
  REQUIRE(std::count(ls.begin(), ls.end(), path("README.md")));

  auto find_hpp = path::find_recursive(".", "*.hpp");
  REQUIRE(std::count(find_hpp.begin(), find_hpp.end(), path("src/util/file.hpp")));
  REQUIRE(std::count(find_hpp.begin(), find_hpp.end(), path("src/util/file_test.cpp")) == 0);

  auto find_cpp = path::find_recursive(".", "*.cpp");
  REQUIRE(std::count(find_cpp.begin(), find_cpp.end(), path("src/util/file.hpp")) == 0);
  REQUIRE(std::count(find_cpp.begin(), find_cpp.end(), path("src/util/file_test.cpp")));
}
