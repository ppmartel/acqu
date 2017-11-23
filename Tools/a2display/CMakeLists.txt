cmake_minimum_required (VERSION 2.6)
include(${CMAKE_SOURCE_DIR}/cmake/subdir-header.cmake)
message(STATUS "Configuring a2display...")

include_directories("inc")

ROOT_PREPARE_LIB("src" "inc" "inc" "Dict.cc" SRCS EXESRCS)

add_library(a2display SHARED ${SRCS})
target_link_libraries(a2display ${ROOT_LIBRARIES})

add_executable(a2display_example examples/a2display_example.cpp)
target_link_libraries(a2display_example a2display)
