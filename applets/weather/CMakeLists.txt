add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.weather\")

install(FILES wind-arrows.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/weather/)

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
        locationlistmodel.cpp
        types.h
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)

target_link_libraries(org.kde.plasma.weather PRIVATE
    Plasma::Plasma
    Plasma::Plasma5Support
    KF6::UnitConversion
    KF6::I18n
    Qt::Qml
)
