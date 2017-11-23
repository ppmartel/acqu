cmake_minimum_required (VERSION 2.6)
include(${CMAKE_SOURCE_DIR}/cmake/subdir-header.cmake)
message(STATUS "Configuring Worker/AR...")

include_directories("src")

set(MOD "Worker")
ROOT_PREPARE_LIB("src" "src" "src" "src/${MOD}Dict.cc" SRCS EXESRCS)
add_library("lib${MOD}" SHARED ${SRCS})
target_link_libraries("lib${MOD}" ${ROOT_LIBRARIES})
set_target_properties("lib${MOD}" PROPERTIES OUTPUT_NAME ${MOD})

# and we create the Worker binary
add_executable(${MOD} "src/${MOD}.cc")
target_link_libraries(${MOD} "lib${MOD}")
