add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.weather\")

kcoreaddons_add_plugin(plasma_applet_weather SOURCES weatherapplet.cpp INSTALL_NAMESPACE "plasma/applets")

kcoreaddons_desktop_to_json(plasma_applet_weather package/metadata.desktop SERVICE_TYPES plasma-applet.desktop)

target_link_libraries(plasma_applet_weather
    KF5::Plasma
    KF5::UnitConversion
    KF5::I18n
)

install(FILES wind-arrows.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/weather/)

plasma_install_package(package org.kde.plasma.weather)


set(weather_SRCS
    plugin/plugin.cpp
    plugin/util.cpp
    plugin/abstractunitlistmodel.cpp
    plugin/locationlistmodel.cpp
    plugin/servicelistmodel.cpp
)

add_library(weatherplugin SHARED ${weather_SRCS})

target_link_libraries(weatherplugin
    KF5::Plasma
    KF5::UnitConversion
    KF5::I18n
    Qt::Qml
)

install(TARGETS weatherplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/weather)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/weather)
