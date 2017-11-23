
# a2display is needed in Core,
# because AcquDAQ can't be properly disentangled
# from AcquRoot analysis part grrrr
add_subdirectory(a2display)
if(ACQU_DAQ)
  return()
endif()

add_subdirectory(Worker)
# check for external OSCAR
if(NOT DEFINED ENV{OSCAR})
  add_subdirectory(OSCAR)
endif()

# bump the variables to parent scope set by the macro SET_ACQU_USER_VARS
set(ACQU_USER_INCS ${ACQU_USER_INCS} PARENT_SCOPE)
set(ACQU_USER_LIBS ${ACQU_USER_LIBS} PARENT_SCOPE)
