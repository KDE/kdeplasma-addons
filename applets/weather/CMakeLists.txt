add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.weather\")

include_directories(
    ${CMAKE_SOURCE_DIR}/libs
)

set(weatherapplet_SRCS
    weatherapplet.cpp)

add_library(plasma_applet_weather MODULE ${weatherapplet_SRCS})

kcoreaddons_desktop_to_json(plasma_applet_weather package/metadata.desktop)

target_link_libraries(plasma_applet_weather
    plasmaweather
    KF5::Plasma
    KF5::UnitConversion
    KF5::IconThemes
)

install(TARGETS plasma_applet_weather DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/applets)

install(FILES wind-arrows.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/weather/)

plasma_install_package(package org.kde.plasma.weather)


set(weather_SRCS
    plugin/plugin.cpp
    plugin/abstractunitlistmodel.cpp
)

add_library(weatherplugin SHARED ${weather_SRCS})

target_link_libraries(weatherplugin
    KF5::UnitConversion
    KF5::I18n
    Qt5::Qml
)

install(TARGETS weatherplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/weather)
install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/weather)
