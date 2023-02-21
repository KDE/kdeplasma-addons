cmake_minimum_required(VERSION 3.16)

project(kdeplasma-addons)
set(PROJECT_VERSION "5.27.1")
set(PROJECT_VERSION_MAJOR 5)

################# Disallow in-source build #################
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()

set(QT_MIN_VERSION 5.15.0)
set(KF5_MIN_VERSION 5.90.0)
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)
include(KDEPackageAppTemplates)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED
    Core
    Gui
    DBus
    Quick
    Qml
    Widgets
    Test
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    Declarative
    Holidays
    I18n
    KIO
    KCMUtils
    Notifications
    Plasma
    Runner
    Service
    Sonnet
    UnitConversion
    NewStuff
)

if(${QT_MAJOR_VERSION} LESS 6)
    find_package(KF5NetworkManagerQt CONFIG QUIET)
    set_package_properties(KF5NetworkManagerQt PROPERTIES
        DESCRIPTION "Qt wrapper for NetworkManager API"
        PURPOSE "Disable wallpaper update when using metered connections"
        TYPE OPTIONAL
    )
    if(KF5NetworkManagerQt_FOUND)
        set(HAVE_NetworkManagerQt TRUE)
    endif()
endif()

find_package(KF5Purpose CONFIG QUIET)
set_package_properties(KF5Purpose PROPERTIES
    DESCRIPTION "Framework for cross-application services and actions"
    PURPOSE "Needed for QuickShare applet"
    URL "https://commits.kde.org/purpose"
    TYPE RUNTIME
)

find_package(ICU 66.1 COMPONENTS uc i18n)
set_package_properties(ICU
        PROPERTIES DESCRIPTION "Unicode and Globalization support for software applications"
        TYPE OPTIONAL
        PURPOSE "Provides alternate calendar systems that are not available in QCalendar"
        )
if(ICU_FOUND)
    set(HAVE_ICU TRUE)
endif()

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_NO_URL_CAST_FROM_STRING
)

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
)

add_subdirectory(dict)
add_subdirectory(profiles)
add_subdirectory(applets)
add_subdirectory(runners)

add_subdirectory(wallpapers)

add_subdirectory(windowswitchers)
add_subdirectory(desktopswitchers)

add_subdirectory(plasmacalendarplugins)

add_subdirectory(templates)
add_subdirectory(appiumtests)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
