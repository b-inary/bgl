# bgl [![Build Status](https://travis-ci.com/b-inary/bgl.svg?branch=master)](https://travis-ci.com/b-inary/bgl)

A modern graph library for research (under development)

### Requirements

- C++17 compiler (tested on g++9 and clang++8)
- [CMake](https://cmake.org/) (>= version 3.8)
- [Ninja](https://ninja-build.org/)

### Build

```sh
$ git clone --recursive git@github.com:b-inary/bgl.git
$ cd bgl
$ make        # build examples
$ make test   # run tests
$ make doc    # generate documents (require Doxygen, Graphviz)
```

(Since bgl is a header-only library, build is actually not needed)

### Usage

There are several ways to use bgl:

1. For prototyping, the easiest way is to use the playground:

```sh
$ cd playground
$ make  # every "*_main.cpp" file is compiled to an executable
```

2. You can make a new project in `projects` directory. Please refer to `sample-project`.

3. bgl can also be used as a CMake library:

```CMake
add_subdirectory(bgl)
target_link_libraries(<your-target> PRIVATE bgl)
```
