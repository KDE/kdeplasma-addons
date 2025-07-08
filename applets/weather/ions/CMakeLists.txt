# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

# the Ion library
set(ionlib_SRCS
    ion.cpp
    ion.h
)

qt_add_library(weatherion STATIC ${ionlib_SRCS})

set_property(TARGET weatherion PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(weatherion PUBLIC
    weatherdata
    KF6::UnitConversion
    KF6::Holidays
    KF6::I18n
    Qt::Qml
)

# dir name where ion modules will be installed
set(WEATHERION_INSTALL_DIR "ions")

# add define because weatherdatamonitor need to know where to find ions
configure_file(ion_config.h.in ion_config.h)

# redefine KDE_INSTALL_PLUGINDIR variable to install ion plugins in the proper directory
set(KDE_INSTALL_PLUGINDIR ${KDE_INSTALL_PLUGINDIR}/${WEATHERION_INSTALL_DIR})

#add ions
add_subdirectory(wetter.com)
add_subdirectory(bbcukmet)
add_subdirectory(dwd)
add_subdirectory(envcan)
add_subdirectory(noaa)
