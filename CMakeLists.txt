
cmake_minimum_required(VERSION 3.8) # CMAKE_CXX_STANDARD 17
project(bgl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -D_BGL_DIRECTORY=${bgl_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=native")

find_package(Threads REQUIRED)
add_subdirectory(extlib/fmt EXCLUDE_FROM_ALL)
set(CATCH_MAIN ${bgl_SOURCE_DIR}/extlib/catch_main.cpp)

add_library(bgl INTERFACE)
target_include_directories(bgl INTERFACE ${bgl_SOURCE_DIR}/include)
target_link_libraries(bgl INTERFACE ${CMAKE_THREAD_LIBS_INIT} fmt-header-only)

add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(playground EXCLUDE_FROM_ALL)
add_subdirectory(projects EXCLUDE_FROM_ALL)
