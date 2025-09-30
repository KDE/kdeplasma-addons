# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(dwd INSTALL_NAMESPACE "plasma/weather_ions")

target_sources(dwd PRIVATE
    ion_dwd.cpp
    ion_dwd.h
)

ecm_qt_declare_logging_category(dwd
    HEADER dwd_debug.h
    IDENTIFIER WEATHER::ION::DWD
    CATEGORY_NAME org.kde.weather.ion.dwd
    DEFAULT_SEVERITY Info
    DESCRIPTION "German Meteorological Service ion plugin"
    EXPORT PLASMAWORKSPACE
)

target_link_libraries(dwd PRIVATE
    plasmaweatherion
    KF6::KIOCore
)
