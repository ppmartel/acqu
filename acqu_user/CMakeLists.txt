cmake_minimum_required (VERSION 2.6)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/subdir-header.cmake)
message(STATUS "Configuring acqu_user...")

# acqu user incs have been collected in previous subprojects
include_directories("root/src" "root/src-mc" ${ACQU_USER_INCS})

set(MOD "UserRoot")
ROOT_PREPARE_LIB("root/src" "root/src" "root/src/UserLinkDef.h" "root/src/UserDict.cc" SRCS EXESRCS)
add_library("lib${MOD}" SHARED ${SRCS})
target_link_libraries("lib${MOD}" ${ROOT_LIBRARIES} ${ACQU_USER_LIBS} Minuit)
set_target_properties("lib${MOD}" PROPERTIES OUTPUT_NAME ${MOD})


set(MOD "MCUserRoot")
ROOT_PREPARE_LIB("root/src-mc" "root/src-mc" "root/src-mc/MCUserLinkDef.h" "root/src-mc/MCDict.cc" SRCS EXESRCS)
add_library("lib${MOD}" SHARED ${SRCS})
target_link_libraries("lib${MOD}" ${ROOT_LIBRARIES} libAcquMC libAcquRoot libAcquDAQ)
set_target_properties("lib${MOD}" PROPERTIES OUTPUT_NAME ${MOD})

# we add many additional files marked as "headers"
# so they appear automatically in QtCreator
# but sort out dead symlinks
file(GLOB_RECURSE ADDITIONAL_FILES "root/macros/*" "data*/*")
set_source_files_properties(${ADDITIONAL_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
foreach(f ${ADDITIONAL_FILES})
  if(EXISTS ${f})
    list(APPEND ADDITIONAL_FILES_EXISTING ${f})
  endif()
endforeach()

# build finally the two executables which many people need desperately ^-^
# it's enough to add the addtional files to the commonly used AcquRoot
set(MOD "AcquRoot")
add_executable(${MOD} "root/src/Main${MOD}.cc" ${ADDITIONAL_FILES_EXISTING})
target_link_libraries(${MOD} libUserRoot)

set(MOD "AcquHead")
add_executable(${MOD} "root/src/Main${MOD}.cc")
target_link_libraries(${MOD} ${ACQU_USER_LIBS} libUserRoot)

set(MOD "AcquMC")
add_executable(${MOD} "root/src-mc/Main${MOD}.cc")
target_link_libraries(${MOD} libMCUserRoot)
