project(kdeplasma-addons)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalDependPackage)

macro_optional_find_package(KDE4Workspace)
macro_log_feature(KDE4WORKSPACE_FOUND "kdebase workspace" "KDE base workspace libraries" "http://www.kde.org" FALSE "" "Needed for building several Plasma plugins")

macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed for building microblog dataengine")

macro_optional_find_package(KdepimLibs)
macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "http://www.kde.org" FALSE "" "Needed for building several Plasma plugins")
find_package(Boost)

set(LIBATTICA_MIN_VERSION "0.1.1")
macro_optional_find_package(LibAttica)
macro_log_feature(LIBATTICA_FOUND "libattica" "Attica Library" "http://websvn.kde.org/trunk/kdesupport/attica/" FALSE "" "Needed for building the Open Collaboration Services plasma dataengine and applets")

macro_optional_find_package(OpenGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3D graphics applications" "http://mesa3d.sourceforge.net" FALSE "" "STRONGLY RECOMMENDED: The 3D hardware acceleration available through the OpenGL API is used in applications ranging from graphics and modellers to screensavers and video players.")

macro_optional_find_package(Nepomuk)
macro_log_feature(NEPOMUK_FOUND "Nepomuk" "Handles all kinds of metadata on the KDE desktop to provide a semantic desktop" "http://nepomuk.kde.org/" FALSE "" "STRONGLY_RECOMMENDED: Nepomuk is used to provide metadata on objects like files, emails etc. to provide a semantic desktop.")
macro_bool_to_01(NEPOMUK_FOUND HAVE_NEPOMUK)


macro_optional_find_package(QJSON)
macro_log_feature(QJSON_FOUND "QJSon" "Used for various Internet runners, for parsing API responses, and the microblog dataengine" "" FALSE "" "")

macro_optional_find_package(QtOAuth)
macro_log_feature(QTOAUTH_FOUND "QtOAuth" "QtOAuth Library - required to build the Plasma Microblog DataEngine" "https://github.com/ayoy/qoauth" FALSE "" "Needed for building Plasma Microblog DataEngine")

macro_optional_depend_package(Marble "Marble")
if (DEPEND_PACKAGE_Marble)
  macro_optional_find_package(Marble)
else (DEPEND_PACKAGE_Marble)
  set (MARBLE_FOUND FALSE)
endif (DEPEND_PACKAGE_Marble)
macro_log_feature(MARBLE_FOUND "Marble" "Desktop Globe" "http://marble.kde.org" FALSE "" "RECOMMENDED: Marble is used to provide a desktop globe wallpaper for Plasma.")

macro_optional_find_package(DBusMenuQt)
macro_log_feature(DBUSMENUQT_FOUND "DBusMenuQt" "DBusMenu Qt" "https://launchpad.net/libdbusmenu-qt/" FALSE "" "Needed for Unity support in IconTasks.")

if (NEPOMUK_FOUND)
  include_directories( ${SOPRANO_INCLUDE_DIR} ${NEPOMUK_INCLUDE_DIR} )
endif (NEPOMUK_FOUND)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories (${KDE4_INCLUDES}
                     ${KDE4WORKSPACE_INCLUDE_DIR}
                     ${KDEPIMLIBS_INCLUDE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}/libs)

add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)

add_subdirectory(scriptengines)
add_subdirectory(wallpapers)
add_subdirectory(containments)
add_subdirectory(cmake)

macro_display_feature_log()
