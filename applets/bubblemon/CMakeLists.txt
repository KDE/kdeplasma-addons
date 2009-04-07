project(plasma-applet-bubblemon)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BUILD_DIR})

add_subdirectory(src)
add_subdirectory(images)

install(FILES plasma-applet-bubblemon.desktop DESTINATION ${SERVICES_INSTALL_DIR})
