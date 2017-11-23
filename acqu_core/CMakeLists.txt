cmake_minimum_required (VERSION 2.6)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/subdir-header.cmake)
message(STATUS "Configuring acqu_core...")

# LZMA for reading xz files directly
# Note that ROOT provides $ROOTSYS/lib/liblzma.a in his own lib
# directory. So you need to rebuild ROOT after
# installing the LZMA header dev files...
find_package(LibLZMA)
if(LIBLZMA_FOUND)
  message(STATUS "Found LibLZMA: Enabling DirectIO with XZ compression")
  include_directories(${LIBLZMA_INCLUDE_DIRS})
  add_definitions(-DWITH_LIBLZMA)
else()
  message(STATUS "No LibLZMA found: DirectIO with XZ compression NOT supported")
endif()

# the following feature helps us to get rid of
# getenv("acqu"), getenv("acqu_sys"), getenv("ROOTSYS")
# because we define this variables now at "configure" time!
set(ACQU_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/AcquRoot/src/CMakeConfig.h")
configure_file("${CMAKE_SOURCE_DIR}/cmake/CMakeConfig.h.in" "${ACQU_CONFIG_FILE}")

# add a precompiler flag to indicate that 
# actual VME access is enabled (this allows running AcquDAQ on non-VME machines)
site_name(MY_HOSTNAME)
if(MY_HOSTNAME MATCHES "^vme-")
  add_definitions(-DVME_HOST)
endif()

# use all "src" subfolders as include directories
# this is also used by ROOT_PREPARE_LIB as arguments to rootcint 
file(GLOB ALLSRCDIRS "*/src")
include_directories(${ALLSRCDIRS} "${CMAKE_CURRENT_BINARY_DIR}/AcquRoot/src" )

# This builds automagically all the ROOT-enabled libraries
macro(CompileCoreLib MOD)
  # this is standard, except that we change the naming
  # of the target. all the magic happens in ROOT_PREPARE_LIB
  ROOT_PREPARE_LIB("${MOD}/src" "${MOD}/src" "${MOD}/src"
    "${MOD}Dict.cc" SRCS EXESRCS)
  add_library("lib${MOD}" SHARED ${SRCS} ${ACQU_CONFIG_FILE})

  set_target_properties("lib${MOD}"
    PROPERTIES COMPILE_FLAGS "${ARGN}" OUTPUT_NAME ${MOD})
  target_link_libraries("lib${MOD}" ${ROOT_LIBRARIES} ${EXTRA_LIBS})
  # track this library for the following exe's
  list(APPEND CORE_LIBS "lib${MOD}")
  list(APPEND CORE_EXESRCS ${EXESRCS})
endmacro()

# some variables to track and configure
# the following macro calls
set(EXTRA_LIBS EG Foam rt)
set(CORE_EXESRCS)
set(CORE_LIBS)

include_directories(${CMAKE_SOURCE_DIR}/Tools/a2display/inc)
CompileCoreLib(AcquRoot)
# link against some optional libraries
if(LIBLZMA_FOUND)
  target_link_libraries(libAcquRoot ${LIBLZMA_LIBRARIES})
endif()

target_link_libraries(libAcquRoot a2display)

CompileCoreLib(AcquDAQ "-O0") # AcquDAQ must not be optimized (hardware access gets scrambled)
CompileCoreLib(AcquMC)

# EPICS/libezca is only included in DAQ mode and if EPICS is found
# if DAQ, look for EPICS
# if not found, no support is built in, thus is not required here
if(ACQU_DAQ)
  find_package(EPICS)
  # this sets many EPICS_* variables used in acqu_core
  if(EPICS_FOUND)
    include_directories(${EPICS_INCLUDES})
    link_directories(${EPICS_LIBRARY_DIR})
    add_definitions(-DACQUROOT_EPICS)
    list(APPEND EXTRA_LIBS ${EPICS_LIBRARIES})
    CompileCoreLib(ezcaRoot)
  endif()
endif()


# install some scripts to the bin directory
# by creating symlinks
file(GLOB CORE_EXTRA_SCRIPTS "scripts/*")
# also add a custom command which deletes all output binaries
#add_custom_target(_link_dummy
#  COMMAND ${CMAKE_COMMAND} -E remove_directory ${EXECUTABLE_OUTPUT_PATH}
#  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXECUTABLE_OUTPUT_PATH}
#  )
#set_source_files_properties(_link_dummy PROPERTIES SYMBOLIC ON)
foreach(f ${CORE_EXTRA_SCRIPTS})
  get_filename_component(f_name ${f} NAME)
  add_custom_target(link_${f_name} ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${f}" "${EXECUTABLE_OUTPUT_PATH}/${f_name}"
    )
endforeach()

# now build the binaries (but only if DAQ mode is on!),
# the libs EG and Foam are yet another ROOT libraries
if(ACQU_DAQ)
  foreach(exesrc ${CORE_EXESRCS})
    # guess the executable name from the source file.
    # Remove leading "Main" (if any) and trailing ".cc"
    # but not for MainHV which builds to AcquHV...args!
    get_filename_component(EXENAME ${exesrc} NAME_WE)
    STRING(REGEX REPLACE "^MainHV$" "AcquHV" EXENAME "${EXENAME}")
    STRING(REGEX REPLACE "^Main(.+)" "\\1" EXENAME "${EXENAME}")
    
    # add the exe as target, only in DAQ mode
    # so targets here do not to interfere with
    # targets in acqu_user
    add_executable(${EXENAME} ${exesrc})
    set_target_properties(${EXENAME}
      PROPERTIES OUTPUT_NAME ${EXENAME})
    target_link_libraries(${EXENAME} ${CORE_LIBS})
  endforeach()
else()
  # in non-DAQ mode, just track the incs/libs for acqu_user
  SET_ACQU_USER_VARS("${CORE_LIBS}")
endif()



