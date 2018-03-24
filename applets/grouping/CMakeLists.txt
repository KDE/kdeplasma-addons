add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.private.grouping\")

plasma_install_package(package org.kde.plasma.private.grouping)

ecm_qt_declare_logging_category(groupingplasmoid_SRCS HEADER debug.h
                                            IDENTIFIER GROUPING_DEBUG
                                            CATEGORY_NAME kde.grouping
                                            DEFAULT_SEVERITY Info)

add_library(org.kde.plasma.private.grouping MODULE groupingcontainment.cpp ${groupingplasmoid_SRCS})

kcoreaddons_desktop_to_json(org.kde.plasma.private.grouping package/metadata.desktop)

target_link_libraries(org.kde.plasma.private.grouping
                      Qt5::Gui
                      Qt5::Quick
                      Qt5::DBus
                      Qt5::Widgets
                      KF5::Plasma
                      KF5::XmlGui
                      KF5::I18n)

install(TARGETS org.kde.plasma.private.grouping DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

add_subdirectory(container)
