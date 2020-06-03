cmake_minimum_required(VERSION 3.0)

project(kdeplasma-addons)
set(PROJECT_VERSION "5.19.0")
set(PROJECT_VERSION_MAJOR 5)

################# Disallow in-source build #################
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()

set(QT_MIN_VERSION 5.12.0)
set(KF5_MIN_VERSION 5.70.0)

include(FeatureSummary)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)
include(KDEPackageAppTemplates)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(KDEClangFormat)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED
    Core
    Gui
    DBus
    Quick
    Qml
    Widgets
    Test
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    CoreAddons
    Declarative
    Holidays
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

find_package(KF5Purpose CONFIG QUIET)
set_package_properties(KF5Purpose PROPERTIES
    DESCRIPTION "Framework for cross-application services and actions"
    PURPOSE "Needed for QuickShare applet"
    URL "https://commits.kde.org/purpose"
    TYPE RUNTIME
)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_NO_URL_CAST_FROM_STRING
)

#########################################################################

################# list the subdirectories #################
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)

add_subdirectory(wallpapers)

add_subdirectory(windowswitchers)
add_subdirectory(desktopswitchers)

add_subdirectory(plasmacalendarplugins)

add_subdirectory(templates)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
