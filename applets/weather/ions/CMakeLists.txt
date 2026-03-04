# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

# the Ion library

add_library(plasmaweatherion SHARED)

target_sources(plasmaweatherion PRIVATE
    ion.cpp
    ion.h
)

target_link_libraries(plasmaweatherion PUBLIC
    plasmaweatherdata
    KF6::UnitConversion
    KF6::Holidays
    KF6::I18n
    Qt::Qml
)

set_target_properties(plasmaweatherion PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME Weather::Ion
)

generate_export_header(plasmaweatherion)

install(TARGETS plasmaweatherion EXPORT plasmaweatherTargets DESTINATION ${KDE_INSTALL_LIBDIR} INCLUDES DESTINATION include/plasma/weather)
install(FILES
        ion.h
        ${CMAKE_CURRENT_BINARY_DIR}/plasmaweatherion_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/plasma/weather
    COMPONENT Devel
)

#add ions
add_subdirectory(wetter.com)
add_subdirectory(bbcukmet)
add_subdirectory(dwd)
add_subdirectory(envcan)
add_subdirectory(noaa)
