# cmake macro to see if we have libLancelot

# LANCELOT_INCLUDE_DIR
# LANCELOT_FOUND
# Copyright (C) 2007 Brad Hards <bradh@frogmouth.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (LANCELOT_INCLUDE_DIR AND LANCELOT_LIBS)
   # Already in cache, be silent
   # This probably means that libLancelot is a part
   # of current build or that this script was
   # already invoked
   set(Lancelot_FIND_QUIETLY TRUE)
   message("Lancelot variables already set")
endif (LANCELOT_INCLUDE_DIR AND LANCELOT_LIBS)

if (NOT Lancelot_FIND_QUIETLY)
   message("Searching for Lancelot")

   find_path(LANCELOT_INCLUDE_DIR NAMES lancelot/lancelot.h
      PATHS
      ${KDE4_INCLUDE_DIR}
      ${INCLUDE_INSTALL_DIR}
   )

   find_library(LANCELOT_LIBS NAMES lancelot
      PATHS
      ${KDE4_LIB_DIR}
      ${LIB_INSTALL_DIR}
   )

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lancelot DEFAULT_MSG LANCELOT_LIBS LANCELOT_INCLUDE_DIR )

   mark_as_advanced(LANCELOT_INCLUDE_DIR LANCELOT_LIBS)
endif (NOT Lancelot_FIND_QUIETLY)

message("-- Found Lancelot include dir: ${LANCELOT_INCLUDE_DIR}")
message("-- Found Lancelot lib: ${LANCELOT_LIBS}")
