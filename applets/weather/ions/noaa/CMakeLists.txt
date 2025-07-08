# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

set(noaaplugin_SRCS
    ion_noaa.cpp
    ion_noaa.h
)

ecm_qt_declare_logging_category(noaaplugin_SRCS
    HEADER noaa_debug.h
    IDENTIFIER WEATHER::ION::NOAA
    CATEGORY_NAME org.kde.weather.ion.noaa
    DEFAULT_SEVERITY Info
    DESCRIPTION "NOAA National Weather Service ion plugin"
    EXPORT PLASMAWORKSPACE
)

qt_add_plugin(noaa
    SHARED
    CLASS_NAME NOAAIon
    ${noaaplugin_SRCS}
)

target_link_libraries(noaa PRIVATE
    weatherion
    KF6::KIOCore
)

install(TARGETS noaa DESTINATION ${KDE_INSTALL_PLUGINDIR})
install (FILES noaa_station_list.xml DESTINATION ${KDE_INSTALL_DATADIR}/plasma/weather)
