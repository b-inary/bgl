
project(examples)
add_custom_target(${PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bgl_SOURCE_DIR}/bin)

file(GLOB SOURCES *.cpp)

foreach(SOURCE_FILE ${SOURCES})
  string(REGEX MATCH "([^/]+).cpp" NAME ${SOURCE_FILE})
  set(RUNTIME_NAME "examples-${CMAKE_MATCH_1}")
  add_executable(${RUNTIME_NAME} ${SOURCE_FILE})
  target_link_libraries(${RUNTIME_NAME} PRIVATE bgl)
  set_target_properties(${RUNTIME_NAME} PROPERTIES RUNTIME_OUTPUT_NAME ${CMAKE_MATCH_1})
  add_dependencies(${PROJECT_NAME} ${RUNTIME_NAME})
endforeach()
