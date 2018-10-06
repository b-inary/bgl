
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bgl_SOURCE_DIR}/bin)

include_directories(${bgl_SOURCE_DIR})
include_directories(${bgl_SOURCE_DIR}/src)

file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} *_test.cpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
list(REMOVE_ITEM SOURCES ${TEST_SOURCES})

add_library(bgl STATIC ${SOURCES})
add_executable(run_tests ${TEST_SOURCES} ${bgl_SOURCE_DIR}/extlib/catch_main.cpp)
target_link_libraries(run_tests bgl)
