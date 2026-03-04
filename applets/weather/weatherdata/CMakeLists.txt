# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt_add_library(plasmaweatherdata SHARED ${weatherdata_SRCS})

target_sources(plasmaweatherdata PRIVATE
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

generate_export_header(plasmaweatherdata)

target_link_libraries(plasmaweatherdata PRIVATE
    Qt::Core
    KF6::I18n
    KF6::UnitConversion
    Qt::Qml
)

set_target_properties(plasmaweatherdata PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME Weather::Data
)

install(TARGETS plasmaweatherdata EXPORT plasmaweatherTargets DESTINATION ${KDE_INSTALL_LIBDIR} INCLUDES DESTINATION include/plasma/weather)
install(FILES
        currentday.h
        lastday.h
        lastobservation.h
        metadata.h
        futuredays.h
        station.h
        warnings.h
        forecast.h
        locations.h
        ${CMAKE_CURRENT_BINARY_DIR}/plasmaweatherdata_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/plasma/weather
    COMPONENT Devel
)
