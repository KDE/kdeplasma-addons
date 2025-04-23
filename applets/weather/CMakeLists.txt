add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.weather\")

install(FILES wind-arrows.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/weather/)
plasma_install_package(package org.kde.plasma.weather)

ecm_add_qml_module(weatherplugin URI org.kde.plasma.private.weather)

target_sources(weatherplugin PRIVATE
    plugin/plugin.cpp
    plugin/util.cpp
    plugin/abstractunitlistmodel.cpp
    plugin/locationlistmodel.cpp
)
target_link_libraries(weatherplugin PRIVATE
    Plasma::Plasma
    Plasma::Plasma5Support
    KF6::UnitConversion
    KF6::I18n
    Qt::Qml
)
ecm_finalize_qml_module(weatherplugin)
