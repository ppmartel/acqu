cmake_minimum_required (VERSION 2.6)
include(${CMAKE_SOURCE_DIR}/cmake/subdir-header.cmake)
message(STATUS "Configuring OSCAR...")

# use all "src" subfolders as include directories
# this is also used by ROOT_PREPARE_LIB as arguments to rootcint 
file(GLOB INCDIRS "*/include")
include_directories(${INCDIRS})

# the big thing here is the libOSCAR, needed by CaLib
set(MOD "OSCAR")

file(GLOB SRCDIRS "*/src")
ROOT_PREPARE_LIB("${SRCDIRS}" "${INCDIRS}" "main/include"
  "main/src/G__${MOD}.cxx" SRCS EXESRCS)
add_library("lib${MOD}" SHARED ${SRCS})
target_link_libraries("lib${MOD}" ${ROOT_LIBRARIES} EG)
set_target_properties("lib${MOD}" PROPERTIES OUTPUT_NAME ${MOD})
if (ROOT_VERSION_MAJOR VERSION_EQUAL 6)
  add_custom_command(TARGET lib${MOD} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/main/src/G__${MOD}_rdict.pcm" ${PROJECT_BINARY_DIR}/lib
  )
endif()

# and we create the oscar-config, for completeness (but not in "all" target)
add_executable("oscar-config" EXCLUDE_FROM_ALL "main/src/Main${MOD}Config.cxx" )
target_link_libraries("oscar-config" "lib${MOD}")

# now track the incs/libs for acqu_user
SET_ACQU_USER_VARS("lib${MOD}")
