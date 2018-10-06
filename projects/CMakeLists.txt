
# add all sub-directories

file(GLOB DIRS *)
file(GLOB NOT_DIRS LIST_DIRECTORIES false *)
list(REMOVE_ITEM DIRS ${NOT_DIRS})

foreach(DIR ${DIRS})
  add_subdirectory(${DIR})
endforeach()
