project(binaryclock)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(binaryclock_SRCS binaryclock.cpp)

kde4_add_ui_files(binaryclock_SRCS clockConfig.ui)
kde4_add_plugin(plasma_applet_binaryclock ${binaryclock_SRCS})
target_link_libraries(plasma_applet_binaryclock
                      ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_binaryclock
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-binaryclock.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

