add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.grouping\")

plasma_add_applet(org.kde.plasma.grouping
    QML_SOURCES
        qml/CompactApplet.qml
        qml/config.qml
        qml/main.qml
        qml/AbstractItem.qml
        qml/PlasmoidItem.qml
    CPP_SOURCES
        groupingcontainment.cpp
    RESOURCES
        main.xml
)
target_link_libraries(org.kde.plasma.grouping PRIVATE
    Qt::Gui
    Qt::Quick
    Qt::DBus
    Qt::Widgets
    Plasma::Plasma
    KF6::XmlGui
    KF6::I18n
)
