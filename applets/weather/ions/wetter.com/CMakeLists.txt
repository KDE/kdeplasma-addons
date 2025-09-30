# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(wettercom INSTALL_NAMESPACE "plasma/weather_ions")

target_sources(wettercom PRIVATE
    ion_wettercom.cpp
    ion_wettercom.h
)

ecm_qt_declare_logging_category(wettercom
    HEADER wettercom_debug.h
    IDENTIFIER WEATHER::ION::WETTERCOM
    CATEGORY_NAME org.kde.weather.ion.wettercom
    DEFAULT_SEVERITY Info
    DESCRIPTION "wetter.com ion plugin"
    EXPORT PLASMAWORKSPACE
)

target_link_libraries(wettercom PRIVATE
    plasmaweatherion
    KF6::KIOCore
)
