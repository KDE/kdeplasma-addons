# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(envcan INSTALL_NAMESPACE "plasma/weather_ions")

target_sources(envcan PRIVATE
    ion_envcan.cpp
    ion_envcan.h
)

ecm_qt_declare_logging_category(envcan
    HEADER envcan_debug.h
    IDENTIFIER WEATHER::ION::ENVCAN
    CATEGORY_NAME org.kde.weather.ion.envcan
    DEFAULT_SEVERITY Info
    DESCRIPTION "Canadian Weather ion plugin"
    EXPORT PLASMAWORKSPACE
)

target_link_libraries(envcan PRIVATE
    plasmaweatherion
    KF6::KIOCore
)
