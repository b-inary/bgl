
cmake_minimum_required(VERSION 3.8) # CMAKE_CXX_STANDARD 17
project(bgl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -D_BGL_DIRECTORY=${bgl_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=native")

include_directories(${bgl_SOURCE_DIR})
include_directories(${bgl_SOURCE_DIR}/extlib/fmt/include)
include_directories(${bgl_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
set(EXT_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} fmt-header-only)
set(TEST_LIBRARY ${bgl_SOURCE_DIR}/extlib/catch_main.cpp)

add_subdirectory(extlib/fmt)
add_subdirectory(test)
add_subdirectory(playground EXCLUDE_FROM_ALL)
add_subdirectory(projects EXCLUDE_FROM_ALL)
