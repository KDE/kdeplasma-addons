# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

set(wettercomplugin_SRCS
    ion_wettercom.cpp
    ion_wettercom.h
)

ecm_qt_declare_logging_category(wettercomplugin_SRCS
    HEADER wettercom_debug.h
    IDENTIFIER WEATHER::ION::WETTERCOM
    CATEGORY_NAME org.kde.weather.ion.wettercom
    DEFAULT_SEVERITY Info
    DESCRIPTION "wetter.com ion plugin"
    EXPORT PLASMAWORKSPACE
)

qt_add_plugin(wettercom
    SHARED
    CLASS_NAME WetterComIon
    ${wettercomplugin_SRCS}
)

target_link_libraries(wettercom PRIVATE
    weatherion
    KF6::KIOCore
)

install(TARGETS wettercom DESTINATION ${KDE_INSTALL_PLUGINDIR})
