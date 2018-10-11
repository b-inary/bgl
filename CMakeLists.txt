
cmake_minimum_required(VERSION 3.8) # CMAKE_CXX_STANDARD 17
project(bgl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# filesystem support
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(FILESYSTEM stdc++fs)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(FILESYSTEM c++fs)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extlib/fmt/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(EXT_LIBRARIES fmt)
set(TEST_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/extlib/catch_main.cpp)

add_subdirectory(extlib/fmt)
add_subdirectory(src)
add_subdirectory(playground EXCLUDE_FROM_ALL)
add_subdirectory(projects EXCLUDE_FROM_ALL)
