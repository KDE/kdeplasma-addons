project(kdeplasma-addons)
set(PROJECT_VERSION "5.1.1")
set(PROJECT_VERSION_MAJOR 5)

cmake_minimum_required(VERSION 2.8.12)

################# Disallow in-source build #################
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()

include(FeatureSummary)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

set(REQUIRED_QT_VERSION 5.3.0)
set(KF5_MIN_VERSION 5.0.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core DBus Quick Qml Widgets)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config ConfigWidgets CoreAddons I18n KIO KCMUtils Plasma Runner Service UnitConversion KDELibs4Support)

#########################################################################

################# list the subdirectories #################
#add_subdirectory(src)
#add_subdirectory( desktoptheme )

# add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)

add_subdirectory(wallpapers)


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
