cmake_minimum_required (VERSION 2.6)
include(${CMAKE_SOURCE_DIR}/cmake/subdir-header.cmake)
message(STATUS "Configuring a2display...")

include_directories("inc")

ROOT_PREPARE_LIB("src" "inc" "inc" "Dict.cc" SRCS EXESRCS)

add_library(a2display SHARED ${SRCS})
target_link_libraries(a2display ${ROOT_LIBRARIES})

if (ROOT_VERSION_MAJOR VERSION_EQUAL 6)
  add_custom_command(TARGET a2display POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/inc/Dict_rdict.pcm" ${PROJECT_BINARY_DIR}/lib
  )
endif()

add_executable(a2display_example examples/a2display_example.cpp)
target_link_libraries(a2display_example a2display)
