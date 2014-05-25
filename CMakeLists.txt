project(kdeplasma-addons)

cmake_minimum_required(VERSION 2.8.10.1)

################# Disallow in-source build #################
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()

include(FeatureSummary)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick X11Extras DBus)

############### Load the CTest options ###############
# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
# configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake COPYONLY)

################# now find all used packages #################

set (QT_MIN_VERSION "5.0.0")


find_package(KF5 REQUIRED COMPONENTS
    IdleTime ItemModels WidgetsAddons WindowSystem Codecs Archive CoreAddons Solid ThreadWeaver
    Config Auth JS Wallet DBusAddons
    I18n GuiAddons Service ItemViews Notifications ConfigWidgets IconThemes Completion JobWidgets Sonnet TextWidgets XmlGui Crash
    UnitConversion Bookmarks KIO Plasma Declarative NewStuff KDE4Support)

find_package(Phonon4Qt5 REQUIRED NO_MODULE)

#optional features
find_package(X11 MODULE)
if(X11_FOUND)
  #X11_Xrender discovery is done by FindX11
  add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
  add_feature_info("X Screensaver Extension (libXss)" X11_Xscreensaver_FOUND "Support for KIdleTime (fallback mode)")
  add_feature_info("X Sync Extension (libXext)" X11_XSync_FOUND "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

  #X11 Session Management (SM) is required
  #X11_SM_FOUND is set in FindX11, which is required by KDE4Internal
  if(UNIX AND NOT X11_SM_FOUND)
    message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
  endif()
endif()

find_package(Freetype)
set_package_properties(Freetype PROPERTIES DESCRIPTION "A font rendering engine"
                       URL "http://www.freetype.org"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build kfontinst, a simple font installer."
                      )

find_package(OpenSSL MODULE)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
                       URL "http://openssl.org"
                       TYPE RECOMMENDED
                       PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
                      )

# find_package(Libintl MODULE)
set_package_properties(Libintl PROPERTIES DESCRIPTION "Support for multiple languages"
                       URL "http://www.gnu.org/software/gettext"
                       TYPE RECOMMENDED
                       PURPOSE "Enables KDE to be available in many different languages"
                      )

#FIXME: when we have a qca for qt5, reenable
# find_package(QCA2 2.0.0 MODULE)
set_package_properties(QCA2 PROPERTIES DESCRIPTION "Support for remote plasma widgets"
                       URL "http://delta.affinix.com/qca"
                       TYPE OPTIONAL
                      )

	      #find_package(DBusMenuQt MODULE)
	      #set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
	      #         URL "https://launchpad.net/libdbusmenu-qt"
	      #         TYPE REQUIRED
	      #        )

#########################################################################

################# list the subdirectories #################
#add_subdirectory(src)
#add_subdirectory( desktoptheme )

# add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
# add_subdirectory(runners)

add_subdirectory(scriptengines)
add_subdirectory(wallpapers)


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
