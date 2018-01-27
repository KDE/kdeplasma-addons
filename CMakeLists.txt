project(kdeplasma-addons)
set(PROJECT_VERSION "5.12.80")
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
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(GenerateExportHeader)

set(REQUIRED_QT_VERSION 5.9.0)
set(KF5_MIN_VERSION 5.42.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Gui DBus Quick Qml Widgets X11Extras)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    CoreAddons
    I18n
    IconThemes
    KIO
    KCMUtils
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

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace"
                      )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
#add_subdirectory(src)
#add_subdirectory( desktoptheme )

add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)

add_subdirectory(wallpapers)

add_subdirectory(windowswitchers)
add_subdirectory(desktopswitchers)

configure_file(config-kdeplasma-addons.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdeplasma-addons.h)

configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
