
# To avoid conflict, please set distinct project name
project(sample-project)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_custom_target(${PROJECT_NAME})

file(GLOB_RECURSE MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} src/*_main.cpp)
file(GLOB_RECURSE LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)
list(REMOVE_ITEM LIB_SOURCES ${MAIN_SOURCES} ${TEST_SOURCES})

foreach(MAIN_SOURCE_FILE ${MAIN_SOURCES})
  string(REGEX MATCH "([^/]+)_main.cpp" NAME ${MAIN_SOURCE_FILE})
  set(RUNTIME_NAME "${PROJECT_NAME}-${CMAKE_MATCH_1}")
  add_executable(${RUNTIME_NAME} ${MAIN_SOURCE_FILE} ${LIB_SOURCES})
  target_link_libraries(${RUNTIME_NAME} PRIVATE bgl)
  set_target_properties(${RUNTIME_NAME} PROPERTIES RUNTIME_OUTPUT_NAME ${CMAKE_MATCH_1})
  add_dependencies(${PROJECT_NAME} ${RUNTIME_NAME})
endforeach()

list(LENGTH TEST_SOURCES TEST_LENGTH)
if(${TEST_LENGTH})
  set(RUN_TESTS "${PROJECT_NAME}-run_tests")
  add_executable(${RUN_TESTS} ${TEST_SOURCES} ${CATCH_MAIN} ${LIB_SOURCES})
  target_link_libraries(${RUN_TESTS} PRIVATE bgl)
  set_target_properties(${RUN_TESTS} PROPERTIES RUNTIME_OUTPUT_NAME run_tests)
  add_dependencies(${PROJECT_NAME} ${RUN_TESTS})
endif()
