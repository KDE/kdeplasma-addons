project(eyes)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Plasma REQUIRED)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(eyes_SRCS eyes.cpp)

#kde4_add_ui_files(eyes_SRCS eyesConfig.ui)
kde4_add_plugin(plasma_applet_eyes ${eyes_SRCS})
target_link_libraries(plasma_applet_eyes
                      ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_eyes
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-eyes.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES eyes.svgz
	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
