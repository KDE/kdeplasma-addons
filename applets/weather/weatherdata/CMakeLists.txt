# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

set(weatherdata_SRCS
    currentday.h
    currentday.cpp
    lastday.h
    lastday.cpp
    lastobservation.h
    lastobservation.cpp
    metadata.h
    metadata.cpp
    futuredays.h
    futuredays.cpp
    station.h
    station.cpp
    warnings.h
    warnings.cpp
    forecast.h
    forecast.cpp
    locations.h
    locations.cpp
)

qt_add_library(plasmaweatherdata SHARED ${weatherdata_SRCS})

generate_export_header(plasmaweatherdata)

target_link_libraries(plasmaweatherdata PRIVATE
    Qt::Core
    KF6::I18n
    KF6::UnitConversion
    Qt::Qml
)

install(TARGETS plasmaweatherdata DESTINATION ${KDE_INSTALL_LIBDIR})
