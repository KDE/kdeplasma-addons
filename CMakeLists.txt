cmake_minimum_required(VERSION 3.0)

project(kdeplasma-addons)
set(PROJECT_VERSION "5.12.80")
set(PROJECT_VERSION_MAJOR 5)

################# Disallow in-source build #################
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()

include(FeatureSummary)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)
include(GenerateExportHeader)

set(REQUIRED_QT_VERSION 5.9.0)
set(KF5_MIN_VERSION 5.42.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED
    Core
    Gui
    DBus
    Quick
    Qml
    Widgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    CoreAddons
    I18n
    IconThemes
    KIO
    KCMUtils
    Kross
    Notifications
    Plasma
    Runner
    Service
    Sonnet
    UnitConversion
    WindowSystem
    NewStuff
)

find_package(LibTaskManager CONFIG REQUIRED)

find_package(KF5Purpose CONFIG QUIET)
set_package_properties(KF5Purpose PROPERTIES
    DESCRIPTION "Framework for cross-application services and actions"
    PURPOSE "Needed for QuickShare applet"
    URL "https://commits.kde.org/purpose"
    TYPE RUNTIME
)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
#     -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)

#########################################################################

################# list the subdirectories #################
add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)

add_subdirectory(wallpapers)

add_subdirectory(windowswitchers)
add_subdirectory(desktopswitchers)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
