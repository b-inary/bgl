
cmake_minimum_required(VERSION 3.7)
project(bgl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra --std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extlib/fmt/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXT_LIBRARIES fmt-header-only)
set(TEST_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/extlib/catch_main.cpp)

add_subdirectory(extlib/fmt)
add_subdirectory(src)
add_subdirectory(playground EXCLUDE_FROM_ALL)
add_subdirectory(projects EXCLUDE_FROM_ALL)
