
cmake_minimum_required(VERSION 2.8.10.1)

project(kdeplasma-addons)

################# Disallow in-source build #################

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()


# Make CPack available to easy generate binary packages
include(CPack)

include(FeatureSummary)

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package(Qt5Core REQUIRED NO_MODULE)

find_package(Qt5Transitional MODULE)
find_package(Qt5Quick REQUIRED NO_MODULE)
find_package(Qt5X11Extras REQUIRED NO_MODULE)
find_package(kde4support REQUIRED NO_MODULE)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

if(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
   set(KDE_NO_DEPRECATED TRUE)
   set(CMAKE_AUTOMOC_MOC_OPTIONS "-DKDE_NO_DEPRECATED")
endif()

############### Load the CTest options ###############
# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
# configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake COPYONLY)

################# now find all used packages #################

set (QT_MIN_VERSION "5.0.0")

find_package(kdeqt5staging REQUIRED NO_MODULE)

find_package(KF5 MODULE REQUIRED COMPONENTS CMake Compiler InstallDirs
    KIdleTime ItemModels KWidgetsAddons KWindowSystem KCodecs KArchive KCoreAddons Solid ThreadWeaver
    KConfig KAuth KJS KWallet KDBusAddons Sonnet
    KI18n KGuiAddons KService KWidgets ItemViews KNotifications KIconThemes KCompletion KJobWidgets KConfigWidgets KTextWidgets XmlGui KCrash)

find_package(KIO REQUIRED NO_MODULE)

find_package(Plasma REQUIRED NO_MODULE)
find_package(kdeclarative REQUIRED NO_MODULE)

# needed for set_package_properties macro
find_package(KDELibs4 REQUIRED NO_MODULE)
find_package(KDE4Support REQUIRED NO_MODULE)

#optional features
find_package(X11 MODULE)
if(X11_FOUND)
  set(HAVE_X11 X11_FOUND)
  #X11_Xrender discovery is done by FindX11
  add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
  add_feature_info("X Screensaver Extension (libXss)" X11_Xscreensaver_FOUND "Support for KIdleTime (fallback mode)")
  add_feature_info("X Sync Extension (libXext)" X11_XSync_FOUND "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

  if(NOT X11_XSync_FOUND AND NOT X11_Xscreensaver_FOUND)
    message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)\n")
  endif()

  #X11 Session Management (SM) is required
  #X11_SM_FOUND is set in FindX11, which is required by KDE4Internal
  if(UNIX AND NOT X11_SM_FOUND)
    message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
  endif()
endif()

find_package(OpenSSL MODULE)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
                       URL "http://openssl.org"
                       TYPE RECOMMENDED
                       PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
                      )

find_package(Libintl MODULE)
set_package_properties(Libintl PROPERTIES DESCRIPTION "Support for multiple languages"
                       URL "http://www.gnu.org/software/gettext"
                       TYPE RECOMMENDED
                       PURPOSE "Enables KDE to be available in many different languages"
                      )

#FIXME: when we have a qca for qt5, reenable
find_package(QCA2 2.0.0 MODULE)
set_package_properties(QCA2 PROPERTIES DESCRIPTION "Support for remote plasma widgets"
                       URL "http://delta.affinix.com/qca"
                       TYPE OPTIONAL
                      )

find_package(DBusMenuQt MODULE)
set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
                       URL "https://launchpad.net/libdbusmenu-qt"
                       TYPE REQUIRED
                      )

#########################################################################

add_definitions(${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)


################# setup the include directories #################
include_directories( ${kdeqt5staging_INCLUDE_DIRS}
                     ${kdeqt5staging_INCLUDE_DIR}
                     ${KDE4_INCLUDES}
                     ${KF5_INCLUDE_DIRS}  # since e-c-m 0.0.7
                   )

# for including config.h and for includes like <kparts/foo.h>
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/src/plasma/includes)

if(QCA2_FOUND)
    include_directories(
        ${QCA2_INCLUDE_DIR}
    )
endif()


macro(installPackage dir component)
   set(root ${ARGV2})
   set(type ${ARGV3})
   if(NOT root)
      set(root plasma/plasmoids)
   endif()
   if(NOT type)
      set(type applet)
   endif()
   install(DIRECTORY ${dir}/ DESTINATION ${DATA_INSTALL_DIR}/${root}/${component} PATTERN .svn EXCLUDE PATTERN CMakeLists.txt EXCLUDE PATTERN Messages.sh EXCLUDE)
   install(FILES ${dir}/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-${type}-${component}.desktop)
endmacro()

################# list the subdirectories #################
#add_subdirectory(src)
#add_subdirectory( desktoptheme )

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)


# project(kdeplasma-addons)
#
# set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
#
# find_package(KDE4 REQUIRED)
# include (KDE4Defaults)
# include(MacroLibrary)
# include(MacroOptionalDependPackage)
#
# macro_optional_find_package(KDE4Workspace)
# macro_log_feature(KDE4WORKSPACE_FOUND "kdebase workspace" "KDE base workspace libraries" "http://www.kde.org" FALSE "" "Needed for building several Plasma plugins")
#
# macro_optional_find_package(QCA2)
# macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed for building microblog dataengine")
#
# macro_optional_find_package(KdepimLibs)
# macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "http://www.kde.org" FALSE "" "Needed for building several Plasma plugins")
# find_package(Boost)
#
# set(LIBATTICA_MIN_VERSION "0.1.1")
# macro_optional_find_package(LibAttica)
# macro_log_feature(LIBATTICA_FOUND "libattica" "Attica Library" "http://websvn.kde.org/trunk/kdesupport/attica/" FALSE "" "Needed for building the Open Collaboration Services plasma dataengine and applets")
#
# macro_optional_find_package(OpenGL)
# macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3D graphics applications" "http://mesa3d.sourceforge.net" FALSE "" "STRONGLY RECOMMENDED: The 3D hardware acceleration available through the OpenGL API is used in applications ranging from graphics and modellers to screensavers and video players.")
#
# macro_optional_find_package(Nepomuk)
# macro_log_feature(NEPOMUK_FOUND "Nepomuk" "Handles all kinds of metadata on the KDE desktop to provide a semantic desktop" "http://nepomuk.kde.org/" FALSE "" "STRONGLY_RECOMMENDED: Nepomuk is used to provide metadata on objects like files, emails etc. to provide a semantic desktop.")
# macro_bool_to_01(NEPOMUK_FOUND HAVE_NEPOMUK)
#
#
# macro_optional_find_package(QJSON)
# macro_log_feature(QJSON_FOUND "QJSon" "Used for various Internet runners, for parsing API responses, and the microblog dataengine" "" FALSE "" "")
#
# macro_optional_find_package(QtOAuth)
# macro_log_feature(QTOAUTH_FOUND "QtOAuth" "QtOAuth Library - required to build the Plasma Microblog DataEngine" "https://github.com/ayoy/qoauth" FALSE "" "Needed for building Plasma Microblog DataEngine")
#
# macro_optional_depend_package(Marble "Marble")
# if (DEPEND_PACKAGE_Marble)
#   macro_optional_find_package(Marble)
# else (DEPEND_PACKAGE_Marble)
#   set (MARBLE_FOUND FALSE)
# endif (DEPEND_PACKAGE_Marble)
# macro_log_feature(MARBLE_FOUND "Marble" "Desktop Globe" "http://marble.kde.org" FALSE "" "RECOMMENDED: Marble is used to provide a desktop globe wallpaper for Plasma.")
#
# macro_optional_find_package(DBusMenuQt)
# macro_log_feature(DBUSMENUQT_FOUND "DBusMenuQt" "DBusMenu Qt" "https://launchpad.net/libdbusmenu-qt/" FALSE "" "Needed for Unity support in IconTasks.")
#
# if (NEPOMUK_FOUND)
#   include_directories( ${SOPRANO_INCLUDE_DIR} ${NEPOMUK_INCLUDE_DIR} )
# endif (NEPOMUK_FOUND)
#
# add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
# add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
# include_directories (${KDE4_INCLUDES}
#                      ${KDE4WORKSPACE_INCLUDE_DIR}
#                      ${KDEPIMLIBS_INCLUDE_DIR}
#                      ${CMAKE_CURRENT_SOURCE_DIR}/libs)
#
# add_subdirectory(libs)
# add_subdirectory(applets)
# add_subdirectory(dataengines)
# add_subdirectory(runners)
#
# add_subdirectory(scriptengines)
add_subdirectory(wallpapers)
# add_subdirectory(containments)
# add_subdirectory(cmake)
#
# macro_display_feature_log()
