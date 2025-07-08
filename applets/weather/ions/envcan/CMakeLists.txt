# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

set(envcanplugin_SRCS
    ion_envcan.cpp
    ion_envcan.h
)

ecm_qt_declare_logging_category(envcanplugin_SRCS
    HEADER envcan_debug.h
    IDENTIFIER WEATHER::ION::ENVCAN
    CATEGORY_NAME org.kde.weather.ion.envcan
    DEFAULT_SEVERITY Info
    DESCRIPTION "Canadian Weather ion plugin"
    EXPORT PLASMAWORKSPACE
)

qt_add_plugin(envcan
    SHARED
    CLASS_NAME EnvCanadaIon
    ${envcanplugin_SRCS}
)

target_link_libraries(envcan PRIVATE
    weatherion
    KF6::KIOCore
)

install(TARGETS envcan DESTINATION ${KDE_INSTALL_PLUGINDIR})
