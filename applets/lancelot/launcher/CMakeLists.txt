project(LancelotMenu)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}

   ${LANCELOT_INCLUDE_DIR}
   ${LANCELOT_DATAMODELS_INCLUDE_DIR}
)

# Lancelot client files
set(plasma_lancelot_client_SRCS
)

set(lancelot_xml ../application/org.kde.lancelot.xml)
qt4_add_dbus_interface(plasma_lancelot_client_SRCS ${lancelot_xml} lancelot_interface)

set(plasma_applet_lancelot_launcher_SRCS
   ${plasma_lancelot_client_SRCS}
   LancelotApplet.cpp
   LancelotAppletConfig.cpp
   ../application/LancelotConfig.cpp
)

kde4_add_ui_files(
   plasma_applet_lancelot_launcher_SRCS
   LancelotAppletConfigBase.ui
)

kde4_add_ui_files(
   plasma_applet_lancelot_launcher_SRCS
   ../application/ui/LancelotConfigBase.ui
)

kde4_add_plugin(plasma_applet_lancelot_launcher ${plasma_applet_lancelot_launcher_SRCS})

target_link_libraries(
   plasma_applet_lancelot_launcher
   ${KDE4_PLASMA_LIBS}
   ${KDE4_KIO_LIBS}
   ${KDE4_KCMUTILS_LIBS}
   ${LANCELOT_LIBS}
   ${LANCELOT_DATAMODELS_LIBS}
)

install(
   TARGETS plasma_applet_lancelot_launcher
   DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
   FILES plasma-applet-lancelot-launcher.desktop
   DESTINATION ${SERVICES_INSTALL_DIR}
)

