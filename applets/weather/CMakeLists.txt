add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.weather\")

kcoreaddons_add_plugin(org.kde.plasma.weather SOURCES weatherapplet.cpp INSTALL_NAMESPACE "plasma/applets")

target_link_libraries(org.kde.plasma.weather
    KF6::Plasma
    KF6::Plasma5Support
    KF6::UnitConversion
    KF6::I18n
)

install(FILES wind-arrows.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/weather/)
plasma_install_package(package org.kde.plasma.weather)

ecm_add_qml_module(weatherplugin URI org.kde.plasma.private.weather)

target_sources(weatherplugin PRIVATE
    plugin/plugin.cpp
    plugin/util.cpp
    plugin/abstractunitlistmodel.cpp
    plugin/locationlistmodel.cpp
)
target_link_libraries(weatherplugin
    KF6::Plasma
    KF6::Plasma5Support
    KF6::UnitConversion
    KF6::I18n
    Qt::Qml
)
ecm_finalize_qml_module(weatherplugin)
