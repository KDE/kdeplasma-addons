project(LancelotMenu)

find_package(SharedMimeInfo REQUIRED)

# Moved up level
# add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

#########################################################
# Checking for Composite Begin                          #
#########################################################

if(X11_Xrender_FOUND )
   set( LANCELOT_HAVE_COMPOSITING 1 )
endif( X11_Xrender_FOUND )

macro_log_feature(LANCELOT_HAVE_COMPOSITING
   "XRender"
   "Lancelot compositing support"
   "http://www.x.org/"
   FALSE
   ""
   "STRONGLY_RECOMMENDED: XRender extension is required for Lancelot compositing support")


configure_file(config-lancelot.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-lancelot.h )

#########################################################
# Checking for Composite End                            #
#########################################################

include(PlasmaUIMacros)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}

   ${LANCELOT_INCLUDE_DIR}
   ${LANCELOT_DATAMODELS_INCLUDE_DIR}
)

# Lancelot application files
set(plasma_lancelot_application_SRCS
   LancelotApplication.cpp
   LancelotWindow.cpp
   LancelotConfig.cpp
   QtDisplay.cpp
   AboutData.cpp
)

plasmaui_add_ui_files(plasma_lancelot_application_SRCS
   "ui/LancelotWindowBase.xml"
)

# Lancelot client files
set(plasma_lancelot_client_SRCS
)

# Lancelot D-Bus connections
#set(plasma_lancelot_dbus_SRCS)

set(ksmserver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma_lancelot_application_SRCS ${ksmserver_xml} ksmserver_interface)

set(lancelot_xml org.kde.lancelot.xml)
qt4_add_dbus_interface(plasma_lancelot_client_SRCS ${lancelot_xml} lancelot_interface)

qt4_add_dbus_adaptor(plasma_lancelot_application_SRCS org.kde.lancelot.xml
   LancelotApplication.h LancelotApplication)

# Lancelot Menu
set(
   plasma_lancelot_SRCS
   main.cpp

   ${plasma_lancelot_application_SRCS}
)

kde4_add_ui_files(
   plasma_lancelot_SRCS
   ui/LancelotConfigBase.ui
)

kde4_add_executable(lancelot-menu ${plasma_lancelot_SRCS})

target_link_libraries(
   lancelot-menu
   ${KDE4_KIO_LIBS}
   ${KDE4_PLASMA_LIBS}
   ${KDE4_KUTILS_LIBS}
   ${LANCELOT_LIBS}
   ${LANCELOT_DATAMODELS_LIBS}
   )

if(LANCELOT_HAVE_COMPOSITING)
target_link_libraries(lancelot-menu ${X11_Xrender_LIB} ${X11_X11_LIB})
endif(LANCELOT_HAVE_COMPOSITING)

set_target_properties(lancelot-menu PROPERTIES OUTPUT_NAME lancelot)

## DBUS service

configure_file(org.kde.lancelot.service.in
   ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lancelot.service)

# To regenerate .xml file:
# qdbuscpp2xml -M -s LancelotApplication.h -o org.kde.lancelot.xml.new
# And make the needed changes
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lancelot.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR}) # ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services )

########### install applicaiton ###############
install(TARGETS lancelot-menu ${INSTALL_TARGETS_DEFAULT_ARGS} )

# Lancelot Part Applet

set(plasma_applet_lancelot_part_SRCS
   ${plasma_lancelot_models_SRCS}
   parts/LancelotPart.cpp
   parts/PartsMergedModel.cpp
   parts/LancelotPartConfig.cpp
)

kde4_add_ui_files(
   plasma_applet_lancelot_part_SRCS
   parts/LancelotPartConfigBase.ui
)

kde4_add_plugin(plasma_applet_lancelot_part ${plasma_applet_lancelot_part_SRCS})

target_link_libraries(
   plasma_applet_lancelot_part
   ${KDE4_PLASMA_LIBS}
   ${KDE4_KIO_LIBS}
   ${LANCELOT_LIBS}
   ${LANCELOT_DATAMODELS_LIBS}
   )

install(
   TARGETS plasma_applet_lancelot_part
   DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
   FILES parts/plasma-applet-lancelot-part.desktop
   DESTINATION ${SERVICES_INSTALL_DIR}
)

install(
   FILES parts/lancelotpart-mime.xml
   DESTINATION ${XDG_MIME_INSTALL_DIR}
)
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})


# Lancelot Main Applet

set(plasma_applet_lancelot_launcher_SRCS
   ${plasma_lancelot_client_SRCS}
   ${plasma_lancelot_models_SRCS}
   launcher/LancelotApplet.cpp
   launcher/LancelotAppletConfig.cpp
   LancelotConfig.cpp
)

kde4_add_ui_files(
   plasma_applet_lancelot_launcher_SRCS
   launcher/LancelotAppletConfigBase.ui
)

kde4_add_ui_files(
   plasma_applet_lancelot_launcher_SRCS
   ui/LancelotConfigBase.ui
)

kde4_add_plugin(plasma_applet_lancelot_launcher ${plasma_applet_lancelot_launcher_SRCS})

target_link_libraries(
   plasma_applet_lancelot_launcher
   ${KDE4_PLASMA_LIBS}
   ${KDE4_KIO_LIBS}
   ${KDE4_KUTILS_LIBS}
   ${LANCELOT_LIBS}
   ${LANCELOT_DATAMODELS_LIBS}
)

install(
   TARGETS plasma_applet_lancelot_launcher
   DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
   FILES launcher/plasma-applet-lancelot-launcher.desktop
   DESTINATION ${SERVICES_INSTALL_DIR}
)

install(FILES lancelot.notifyrc DESTINATION ${DATA_INSTALL_DIR}/lancelot)
