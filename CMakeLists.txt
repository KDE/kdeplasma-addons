project(kdeplasma-addons)
set(PROJECT_VERSION "5.5.0")
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
set(KF5_MIN_VERSION 5.8.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Gui DBus Quick Qml Widgets X11Extras)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Activities Config ConfigWidgets CoreAddons I18n KIO KCMUtils Plasma Runner Service UnitConversion KDELibs4Support NewStuff Kross)

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace"
                      )

find_package(XCB
    REQUIRED COMPONENTS
        XCB SHM IMAGE
)
set_package_properties(XCB PROPERTIES TYPE REQUIRED)
add_feature_info("XCB-XKB" XCB_XKB_FOUND "Required for building kcm/keyboard")
add_feature_info("libxft" X11_Xft_FOUND "X FreeType interface library required for font installation")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
#########################################################################

################# list the subdirectories #################
#add_subdirectory(src)
#add_subdirectory( desktoptheme )

# add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)

add_subdirectory(wallpapers)

add_subdirectory(windowswitchers)
add_subdirectory(desktopswitchers)

configure_file(config-kdeplasma-addons.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdeplasma-addons.h)

configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
