
project(bgl-test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bgl_SOURCE_DIR}/bin)

file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable(run_tests ${TEST_SOURCES} ${CATCH_MAIN})
target_link_libraries(run_tests PRIVATE bgl)
