cmake_minimum_required (VERSION 2.6)
project(acqu)

# check for in-source build, forbid it!
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR AND NOT MSVC_IDE)
  message(FATAL_ERROR "\nIn-source build attempt detected!\n"
    "Please create a new directory (e.g. build) and run `cmake ..`.\n"
    "Also don't forget to delete the created CMakeCache.txt and CMakeFiles dir"
    )
endif()
    
include (cmake/settings.cmake)

message(STATUS "*** Build Type:      " ${CMAKE_BUILD_TYPE})
message(STATUS "*** Compiler Flags:  " ${DEFAULT_COMPILE_FLAGS})
message(STATUS "*** Install libs to: " ${LIBRARY_OUTPUT_PATH})
message(STATUS "*** Install bin to:  " ${EXECUTABLE_OUTPUT_PATH})
if(ACQU_DAQ)
  message(STATUS "*** Building DAQ Part only (ACQU_DAQ=On)")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})
# require a fairly recent ROOT version
find_package(ROOT "5.30" REQUIRED)

#if(NOT ${ROOT_VERSION_MAJOR} EQUAL 5) 
#  message(FATAL_ERROR "Only ROOT version 5.x.x supported")
#endif() 

# since all subprojects need ROOT, set that up here
include_directories(${ROOT_INCLUDES})
link_directories(${ROOT_LIBRARY_DIR})


# in these two variables, we collect over the subprojects
# the include dirs and the libnames on which acqu_user
# code can depend on...
set(ACQU_USER_INCS "")
set(ACQU_USER_LIBS "")
# this must be a macro to have the same scope as the subdir
macro(SET_ACQU_USER_VARS LIBS)
  get_property(allincdirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY INCLUDE_DIRECTORIES)
  list(APPEND ACQU_USER_INCS ${allincdirs})
  set(ACQU_USER_INCS ${ACQU_USER_INCS} PARENT_SCOPE)
  
  list(APPEND ACQU_USER_LIBS ${LIBS})
  set(ACQU_USER_LIBS ${ACQU_USER_LIBS} PARENT_SCOPE)
endmacro()

# helper libraries
add_subdirectory(Tools)

add_subdirectory(acqu_core)

# ACQU_DAQ=On usually on VME machines for DAQ
if(ACQU_DAQ)
  return()
endif()

# check for external CaLib
if(DEFINED ENV{CALIB})
  message("-- Found external CaLib in " $ENV{CALIB})
else()
  add_subdirectory(CaLib)
endif()

# finally, the main user part
add_subdirectory(acqu_user)

# doxygen stuff
include(cmake/doxygen.cmake)
