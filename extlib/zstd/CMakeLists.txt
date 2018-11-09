
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/common)
add_compile_options(-DXXH_NAMESPACE=ZSTD_ -DZSTD_MULTITHREAD)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR} common/*.c decompress/*.c)
add_library(zstd STATIC ${SOURCES})
