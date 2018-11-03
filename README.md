# bgl [![Build Status](https://travis-ci.com/b-inary/bgl.svg?branch=master)](https://travis-ci.com/b-inary/bgl)

A modern graph library for research

**Requirements**

- C++17 compiler (tested on g++8 and clang++7)
- [CMake](https://cmake.org/) (>= version 3.8)
- [Ninja](https://ninja-build.org/)

**Build**

    $ git clone --recursive git@github.com:b-inary/bgl.git
    $ cd bgl
    $ make test  # run tests
    $ make doc   # generate documents (require Doxygen, Graphviz)

(Since bgl is a header-only library, actually build is not needed)

**Usage**

For prototyping, the easiest way is to use the playground:

    $ cd playground
    $ make  # every "*_main.cpp" file is compiled as executable

Or, you can make a new project in ``projects`` directory.  
Please refer to ``sample-project``.

bgl can also be used as CMake library:

    add_subdirectory(bgl)
    target_link_libraries(<your-project> PRIVATE bgl)
