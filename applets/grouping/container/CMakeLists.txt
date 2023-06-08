plasma_install_package(package org.kde.plasma.grouping)

set(groupedappletscontainer_SRCS
    groupedappletscontainer.cpp
)
ecm_qt_declare_logging_category(groupedappletscontainer_SRCS HEADER debug.h
                                            IDENTIFIER GROUPING_DEBUG
                                            CATEGORY_NAME kde.grouping
                                            DEFAULT_SEVERITY Info)

kcoreaddons_add_plugin(org.kde.plasma.grouping SOURCES ${groupedappletscontainer_SRCS} INSTALL_NAMESPACE "plasma/applets")

target_link_libraries(org.kde.plasma.grouping
                      Qt::Gui
                      Qt::Quick
                      KF6::Plasma
                      KF6::PlasmaQuick
                      KF6::XmlGui
                      KF6::I18n)
