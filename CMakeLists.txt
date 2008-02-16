project(plasma_playground)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroOptionalAddSubdirectory)

find_package(Plasma REQUIRED)

macro_optional_find_package(OpenGL)
macro_optional_find_package(Blitz)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(scriptengines)
add_subdirectory(desktopthemes)

