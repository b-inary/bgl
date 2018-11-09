
cmake_minimum_required(VERSION 3.8) # CMAKE_CXX_STANDARD 17
project(bgl)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile options
add_compile_options(-Wall -Wextra -D_BGL_DIRECTORY=${bgl_SOURCE_DIR})

# force color diagnostics
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

# optimization
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=native")

# prepare libraries
find_package(Threads REQUIRED)
add_subdirectory(extlib/fmt EXCLUDE_FROM_ALL)
add_subdirectory(extlib/zstd EXCLUDE_FROM_ALL)
set(CATCH_MAIN ${bgl_SOURCE_DIR}/extlib/catch_main.cpp)

# set bgl target
add_library(bgl INTERFACE)
target_include_directories(bgl INTERFACE ${bgl_SOURCE_DIR}/include)
target_link_libraries(bgl INTERFACE ${CMAKE_THREAD_LIBS_INIT} fmt-header-only zstd)

# load subdirectories
add_subdirectory(examples EXCLUDE_FROM_ALL)
add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(playground EXCLUDE_FROM_ALL)
add_subdirectory(projects EXCLUDE_FROM_ALL)
