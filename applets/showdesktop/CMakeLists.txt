project(plasma-showdesktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} )

set(showdesktop_SRCS
    showdesktop.cpp
)

kde4_add_plugin(plasma_applet_showdesktop ${showdesktop_SRCS})
target_link_libraries(plasma_applet_showdesktop
     ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} )

install(TARGETS plasma_applet_showdesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-showdesktop.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
