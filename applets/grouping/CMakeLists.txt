project(grouping-plasmoid)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(QT_MIN_VERSION "5.6.1")
set(KF5_MIN_VERSION "5.26.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Widgets Quick DBus)
find_package(ECM 1.8.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
                    Plasma XmlGui I18n IconThemes)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMPackageConfigHelpers)
include(ECMMarkNonGuiExecutable)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckIncludeFiles)
include(FeatureSummary)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(KDEPackageAppTemplates)

#actual stuff to keep after merging

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.private.grouping\")

plasma_install_package(package org.kde.plasma.private.grouping)

set(groupingplasmoid_SRCS
    groupingcontainment.cpp
)

ecm_qt_declare_logging_category(groupingplasmoid_SRCS HEADER debug.h
                                            IDENTIFIER GROUPING_DEBUG
                                            CATEGORY_NAME kde.grouping
                                            DEFAULT_SEVERITY Info)

add_library(org.kde.plasma.private.grouping MODULE ${groupingplasmoid_SRCS})

kcoreaddons_desktop_to_json(org.kde.plasma.private.grouping package/metadata.desktop)

target_link_libraries(org.kde.plasma.private.grouping
                      Qt5::Gui
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::Plasma
                      KF5::IconThemes
                      KF5::XmlGui
                      KF5::I18n)

install(TARGETS org.kde.plasma.private.grouping DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

add_subdirectory(container)
