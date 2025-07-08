# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

set(dwdplugin_SRCS
    ion_dwd.cpp
    ion_dwd.h
)

ecm_qt_declare_logging_category(dwdplugin_SRCS
    HEADER dwd_debug.h
    IDENTIFIER WEATHER::ION::DWD
    CATEGORY_NAME org.kde.weather.ion.dwd
    DEFAULT_SEVERITY Info
    DESCRIPTION "German Meteorological Service ion plugin"
    EXPORT PLASMAWORKSPACE
)

qt_add_plugin(dwd
    SHARED
    CLASS_NAME DWDIon
    ${dwdplugin_SRCS}
)

target_link_libraries(dwd PRIVATE
    weatherion
    KF6::KIOCore
)

install(TARGETS dwd DESTINATION ${KDE_INSTALL_PLUGINDIR})
