# - Find ROOT instalation
# This module tries to find the ROOT installation on your system.
# It tries to find the root-config script which gives you all the needed 
# information.
# If the system variable ROOTSYS is set this is straight forward.
Message(STATUS "Looking for CCTVMEEN...")

# Include dir
find_path(CCTVMEEN_INCLUDE_DIR
  NAMES cctvmeen.h
##PATHS ${ctvmeen_PKGCONF_INCLUDE_DIRS}
)
if(NOT CCTVMEEN_INCLUDE_DIR)
       message(FATAL_ERROR "   cctvmeen header file not found")
else()
       message("   Universe II cctvmeen library = ${CCTVMEEN_INCLUDE_DIR}")
endif()

# Finally the library itself
find_library(CCTVMEEN_LIB
  NAMES cctvmeen
##  PATHS ${cctvmeen_PKGCONF_LIBRARY_DIRS}
)
if(NOT CCTVMEEN_LIB)
       message(FATAL_ERROR "   cctvmeen library not found")
else()
       message("   Universe II cctvmeen library = ${CCTVMEEN_LIB}")
endif()
