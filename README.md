# bgl [![Build Status](https://travis-ci.com/b-inary/bgl.svg?branch=master)](https://travis-ci.com/b-inary/bgl)

research prototyping oriented graph library just for me

**requirements**

- C++17 compiler (tested on g++8 and clang++7)
- [CMake](https://cmake.org/) (>= version 3.8)
- [Ninja](https://ninja-build.org/)

**build**

    git clone --recursive git@github.com:b-inary/bgl.git
    cd bgl
    make
    make test  # run tests
    make doc   # generate documents (require Doxygen, Graphviz)
