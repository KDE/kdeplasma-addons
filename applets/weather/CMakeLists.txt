# SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.weather\")

install(FILES wind-arrows.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/weather/)

add_subdirectory(weatherdata)
add_subdirectory(ions)

plasma_add_applet(org.kde.plasma.weather
    QML_SOURCES
        qml/CompactRepresentation.qml
        qml/DetailsView.qml
        qml/ForecastView.qml
        qml/FullRepresentation.qml
        qml/IconAndTextItem.qml
        qml/main.qml
        qml/NoticesView.qml
        qml/SwitchPanel.qml
        qml/TopPanel.qml
        qml/config/ConfigAppearance.qml
        qml/config/ConfigUnits.qml
        qml/config/ConfigWeatherStation.qml
        qml/config/config.qml
    CPP_SOURCES
        util.cpp
        abstractunitlistmodel.cpp
        locationscontrol.cpp
        forecastcontrol.cpp
        weatherdatamonitor_p.cpp
        abstractunitlistmodel.cpp
        ioncontrol_p.cpp
        forecastdata.cpp
        locationsdata.cpp
        types.h
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)

ecm_qt_declare_logging_category(org.kde.plasma.weather
    HEADER weathercontroller_debug.h
    IDENTIFIER WEATHER::CONTROLLER
    CATEGORY_NAME org.kde.applets.weather.controller
    DEFAULT_SEVERITY Info
    DESCRIPTION "Weather controller"
    EXPORT PLASMAWORKSPACE
)

target_link_libraries(org.kde.plasma.weather PRIVATE
    KF6::UnitConversion
    KF6::I18n
    Qt::Core
    Qt::DBus
    Qt::Qml
    Qt::Network
    Qt::Gui
    KF6::KIOCore
    plasmaweatherion
    plasmaweatherdata
)
