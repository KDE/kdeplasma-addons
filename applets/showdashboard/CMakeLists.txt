project(showdashboard)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(showdashboard_SRCS showdashboard.cpp)

kde4_add_plugin(plasma_applet_showdashboard ${showdashboard_SRCS})
target_link_libraries(plasma_applet_showdashboard
                       ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_showdashboard
         DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-showdashboard.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})

