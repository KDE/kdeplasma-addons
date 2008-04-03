# cmake macro to see if we have Plasma from KDE workspace

# LANCELOT_INCLUDE_DIR
# LANCELOT_FOUND
# Copyright (C) 2007 Brad Hards <bradh@frogmouth.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (LANCELOT_INCLUDE_DIR AND LANCELOT_LIBS)
    # Already in cache, be silent
    set(Lancelot_FIND_QUIETLY TRUE)
endif (LANCELOT_INCLUDE_DIR AND LANCELOT_LIBS)

if (NOT Lancelot_FIND_QUIETLY)
   find_path(LANCELOT_INCLUDE_DIR NAMES lancelot/lancelot.h
      PATHS
      ${INCLUDE_INSTALL_DIR}
   )

   find_library(LANCELOT_LIBS NAMES lancelot
      PATHS
      ${LIB_INSTALL_DIR}
   )

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lancelot DEFAULT_MSG LANCELOT_LIBS LANCELOT_INCLUDE_DIR )

   mark_as_advanced(LANCELOT_INCLUDE_DIR LANCELOT_LIBS)
endif (NOT Lancelot_FIND_QUIETLY)


