plasma_install_package(package org.kde.plasma.grouping)

set(groupedappletscontainer_SRCS
    groupedappletscontainer.cpp
)
ecm_qt_declare_logging_category(groupedappletscontainer_SRCS HEADER debug.h
                                            IDENTIFIER GROUPING_DEBUG
                                            CATEGORY_NAME kde.grouping
                                            DEFAULT_SEVERITY Info)

add_library(org.kde.plasma.grouping MODULE ${groupedappletscontainer_SRCS})

kcoreaddons_desktop_to_json(org.kde.plasma.grouping package/metadata.desktop)


target_link_libraries(org.kde.plasma.grouping
                      Qt5::Gui
                      Qt5::Quick
                      KF5::Plasma
                      KF5::XmlGui
                      KF5::I18n)

install(TARGETS org.kde.plasma.grouping DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)


