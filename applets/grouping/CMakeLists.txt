add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.grouping\")

plasma_install_package(package org.kde.plasma.grouping)
kcoreaddons_add_plugin(org.kde.plasma.grouping SOURCES groupingcontainment.cpp INSTALL_NAMESPACE "plasma/applets")
target_link_libraries(org.kde.plasma.grouping
    Qt::Gui
    Qt::Quick
    Qt::DBus
    Qt::Widgets
    Plasma::Plasma
    Plasma::Plasma5Support
    KF6::XmlGui
    KF6::I18n
)
