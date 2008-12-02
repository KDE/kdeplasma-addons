project(kdeplasma-addons)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroLibrary)

find_package(KDE4Workspace REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(Boost)

macro_optional_find_package(OpenGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3D graphics applications" "http://mesa3d.sourceforge.net" FALSE "" "STRONGLY RECOMMENDED: The 3D hardware acceleration available through the OpenGL API is used in applications ranging from graphics and modellers to screensavers and video players.")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${KDE4_INCLUDES} 
                     ${KDE4WORKSPACE_INCLUDE_DIR}
                     ${KDEPIMLIBS_INCLUDE_DIR} )


add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(scriptengines)
add_subdirectory(desktopthemes)

macro_display_feature_log()
