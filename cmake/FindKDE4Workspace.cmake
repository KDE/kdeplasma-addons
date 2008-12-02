# - Try to find the workspace libraries from KDE4 kdebase
# find_package(KDE4) has to be called before using this module.
# Once done this will define
#
#  KDE4WORKSPACE_FOUND - KDE4 workspace has been found
#  KDE4WORKSPACE_INCLUDE_DIR - the include dir for using the KDE workspace libraries
#  KDE4WORKSPACE_PLASMACLOCK_LIBRARY - the plasmaclock library
#  KDE4WORKSPACE_WEATHERION_LIBRARY - the weather_ion library
#  KDE4WORKSPACE_TASKMANAGER_LIBRARY - the taskmanager library


# Copyright (c) 2008, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(KDE4_FOUND)

   # check for the plasmaclock library from kdebase/workspace
   # currently used in applets/fuzzy-clock and applets/binary-clock, Alex
   find_path(KDE4WORKSPACE_INCLUDE_DIR libplasmaclock/clockapplet.h PATHS ${KDE4_INCLUDE_DIR})

   find_library(KDE4WORKSPACE_PLASMACLOCK_LIBRARY plasmaclock PATHS ${KDE4_LIB_DIR})
   find_library(KDE4WORKSPACE_WEATHERION_LIBRARY weather_ion PATHS ${KDE4_LIB_DIR})
   find_library(KDE4WORKSPACE_TASKMANAGER_LIBRARY taskmanager PATHS ${KDE4_LIB_DIR})

   mark_as_advanced(KDE4WORKSPACE_INCLUDE_DIR 
                    KDE4WORKSPACE_PLASMACLOCK_LIBRARY
                    KDE4WORKSPACE_WEATHERION_LIBRARY
                    KDE4WORKSPACE_TASKMANAGER_LIBRARY)

endif(NOT KDE4_FOUND)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(KDE4WORKSPACE 
                                  "KDE workspace libraries not found, install kdebase/workspace/libs"
                                  KDE4WORKSPACE_INCLUDE_DIR 
                                  KDE4WORKSPACE_PLASMACLOCK_LIBRARY 
                                  KDE4WORKSPACE_WEATHERION_LIBRARY 
                                  KDE4WORKSPACE_TASKMANAGER_LIBRARY)
