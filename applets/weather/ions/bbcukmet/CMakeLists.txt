# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

set(bbcukmetplugin_SRCS
    ion_bbcukmet.cpp
    ion_bbcukmet.h
)

ecm_qt_declare_logging_category(bbcukmetplugin_SRCS
    HEADER bbcukmet_debug.h
    IDENTIFIER WEATHER::ION::BBCUKMET
    CATEGORY_NAME org.kde.weather.ion.bbcukmet
    DEFAULT_SEVERITY Info
    DESCRIPTION "BBC weather ion plugin"
    EXPORT PLASMAWORKSPACE
)

qt_add_plugin(bbcukmet
    SHARED
    CLASS_NAME UKMETIon
    ${bbcukmetplugin_SRCS}
)

target_link_libraries(bbcukmet PRIVATE
    weatherion
    KF6::KIOCore
)

install(TARGETS bbcukmet DESTINATION ${KDE_INSTALL_PLUGINDIR})
