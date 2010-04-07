# Project Needs a name ofcourse
project(plasmaboard)
	 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
	 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
	 
# We add our source code here
set(plasmaboard_SRCS widget.cpp BoardKey.cpp PanelIcon.cpp AlphaNumKey.cpp FuncKey.cpp EnterKey.cpp BackspaceKey.cpp TabKey.cpp ShiftKey.cpp CapsKey.cpp ArrowTopKey.cpp ArrowBottomKey.cpp ArrowLeftKey.cpp ArrowRightKey.cpp tooltip.cpp StickyKey.cpp Layout.cpp)
	 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_plasmaboard ${plasmaboard_SRCS})
target_link_libraries(plasma_applet_plasmaboard 
                ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} kephal plasmaclock ${X11_XTest_LIB} ${X11_X11_LIB} ${X11_LIBS})
 
install(TARGETS plasma_applet_plasmaboard DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES qwert_layout.xml pc105_layout.xml DESTINATION ${DATA_INSTALL_DIR}/plasmaboard )
 
install(FILES plasma_applet_plasmaboard.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
        
#install(FILES plasmaboard_key.svg
#	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
