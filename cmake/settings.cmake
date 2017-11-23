# this is a hard requirement...will be sorted out later
#set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

# make more warnings appear
add_definitions(-D_FORTIFY_SOURCE=2)

# every subdirectory has its own bin/lib path
# this should be changed to one "global" directory...
if(NOT DEFINED EXECUTABLE_OUTPUT_PATH)
	set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
endif()

if(NOT DEFINED LIBRARY_OUTPUT_PATH)
	set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
endif()

# really no optimization in debug mode
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
else()
  message(FATAL_ERROR "Non-gcc compiler not supported")
endif()

# set default build type if unspecified so far
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

# figure out compile flags
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
set(DEFAULT_COMPILE_FLAGS ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})


# disable the DAQ build mode by default
# unless the site_name aka hostname begins with "vme-"
if(NOT DEFINED ACQU_DAQ)
  site_name(MY_HOSTNAME)
  if(MY_HOSTNAME MATCHES "^vme-")
    set(ACQU_DAQ 1)
  else()
    set(ACQU_DAQ 0)
  endif()
else()
  string(TOUPPER "${ACQU_DAQ}" ACQU_DAQ)
  if(NOT "${ACQU_DAQ}" STREQUAL "ON")
    set(ACQU_DAQ 0)
  endif()
endif()

# currently the following variables are used to
# create a configure file in acqu_core

# set the "acqu" version and the date...
if(NOT DEFINED ACQU_VERSION)
  set(ACQU_VERSION "4v6")
endif()

if(NOT DEFINED ACQU_DATE)
  set(ACQU_DATE "09.07.13")
endif()

# ...and the very handy absolute path names...
if(NOT DEFINED ACQU_SYSCORE_PATH)
  get_filename_component(ACQU_SYSCORE_PATH "acqu_core" ABSOLUTE)
endif()

if(NOT DEFINED ACQU_USER_PATH)
  get_filename_component(ACQU_USER_PATH "acqu_user" ABSOLUTE)
endif()
