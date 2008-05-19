project(showdesktop)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Plasma REQUIRED)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(showdesktop_SRCS showdesktop.cpp)

kde4_add_plugin(plasma_applet_showdesktop ${showdesktop_SRCS})
target_link_libraries(plasma_applet_showdesktop
                      ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_showdesktop
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-showdesktop.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

