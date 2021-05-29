add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.private.grouping\")

plasma_install_package(package org.kde.plasma.private.grouping)

ecm_qt_declare_logging_category(groupingplasmoid_SRCS HEADER debug.h
                                            IDENTIFIER GROUPING_DEBUG
                                            CATEGORY_NAME kde.grouping
                                            DEFAULT_SEVERITY Info)

kcoreaddons_add_plugin(org.kde.plasma.private.grouping SOURCES groupingcontainment.cpp ${groupingplasmoid_SRCS} INSTALL_NAMESPACE "plasma/applets")

kcoreaddons_desktop_to_json(org.kde.plasma.private.grouping package/metadata.desktop)

target_link_libraries(org.kde.plasma.private.grouping
                      Qt::Gui
                      Qt::Quick
                      Qt::DBus
                      Qt::Widgets
                      KF5::Plasma
                      KF5::XmlGui
                      KF5::I18n)

add_subdirectory(container)
