#   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2,
#   or (at your option) any later version, as published by the Free
#   Software Foundation
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the
#   Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

# cmake macro to see if we have libLancelot-datamodels

# LANCELOT_DATAMODELS_INCLUDE_DIR
# LANCELOT_DATAMODELS_FOUND
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (LANCELOT_DATAMODELS_INCLUDE_DIR AND LANCELOT_DATAMODELS_LIBS)
   # Already in cache, be silent
   # This probably means that libLanceloti-datamodels is a part
   # of current build or that this script was
   # already invoked
   set(Lancelot_Datamodels_FIND_QUIETLY TRUE)
   message("Lancelot Datamodels variables already set")
endif (LANCELOT_DATAMODELS_INCLUDE_DIR AND LANCELOT_DATAMODELS_LIBS)

if (NOT Lancelot_Datamodels_FIND_QUIETLY)
   message("Searching for Lancelot Datamodels")

   find_path(LANCELOT_DATAMODELS_INCLUDE_DIR NAMES lancelot-datamodels/BaseModel.h
      PATHS
      ${KDE4_INCLUDE_DIR}
      ${INCLUDE_INSTALL_DIR}
   )

   find_library(LANCELOT_DATAMODELS_LIBS NAMES lancelot-datamodels
      PATHS
      ${KDE4_LIB_DIR}
      ${LIB_INSTALL_DIR}
   )

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(LancelotDatamodels DEFAULT_MSG LANCELOT_DATAMODELS_LIBS LANCELOT_DATAMODELS_INCLUDE_DIR )

   mark_as_advanced(LANCELOT_DATAMODELS_INCLUDE_DIR DIR LANCELOT_DATAMODELS_LIBS)
endif (NOT Lancelot_Datamodels_FIND_QUIETLY)

message("-- Found Lancelot Datamodels include dir: ${LANCELOT_DATAMODELS_INCLUDE_DIR}")
message("-- Found Lancelot Datamodels lib: ${LANCELOT_DATAMODELS_LIBS}")
