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
)

# Lancelot data models files
set(plasma_lancelot_models_SRCS
  models/BaseModel.cpp
  models/BaseMergedModel.cpp
  models/FolderModel.cpp
  models/XbelModel.cpp

  models/FavoriteApplications.cpp
  models/Applications.cpp

  models/Devices.cpp
  models/Places.cpp
  models/SystemServices.cpp

  models/RecentDocuments.cpp
  models/OpenDocuments.cpp
  models/NewDocuments.cpp

  models/ContactsKopete.cpp
  models/MessagesKmail.cpp

  models/Runner.cpp
  models/SystemActions.cpp

  logger/Logger.cpp
)

# Lancelot application files
set(plasma_lancelot_application_SRCS
  LancelotApplication.cpp
  LancelotWindow.cpp
  Serializator.cpp
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

set(screensaver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.freedesktop.ScreenSaver.xml)
qt4_add_dbus_interface(plasma_lancelot_models_SRCS ${screensaver_xml} screensaver_interface)

# set(krunner_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.krunner.Interface.xml)
# qt4_add_dbus_interface(plasma_lancelot_application_SRCS ${krunner_xml} krunner_interface)

set(ksmserver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma_lancelot_application_SRCS ${ksmserver_xml} ksmserver_interface)

set(kmail_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.kmail.kmail.xml)
if (NOT EXISTS ${kmail_xml})
  set(kmail_xml external/org.kde.kmail.kmail.xml)
endif (NOT EXISTS ${kmail_xml})
qt4_add_dbus_interface(plasma_lancelot_models_SRCS ${kmail_xml} kmail_interface)

set(kmail_folder_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.kmail.folder.xml)
if (NOT EXISTS ${kmail_folder_xml})
  set(kmail_folder_xml external/org.kde.kmail.folder.xml)
endif (NOT EXISTS ${kmail_folder_xml})
qt4_add_dbus_interface(plasma_lancelot_models_SRCS ${kmail_folder_xml} kmailfolder_interface)

set(kopete_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.Kopete.xml)
if (NOT EXISTS ${kopete_xml})
  set(kopete_xml external/org.kde.Kopete.xml)
endif (NOT EXISTS ${kopete_xml})
qt4_add_dbus_interface(plasma_lancelot_models_SRCS ${kopete_xml} kopete_interface)

set(lancelot_xml org.kde.lancelot.xml)
qt4_add_dbus_interface(plasma_lancelot_client_SRCS ${lancelot_xml} lancelot_interface)

qt4_add_dbus_adaptor(plasma_lancelot_application_SRCS org.kde.lancelot.xml
     LancelotApplication.h LancelotApplication)

# Lancelot Menu
set(
  plasma_lancelot_SRCS
  main.cpp

  ${plasma_lancelot_models_SRCS}
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
    ${LANCELOT_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4WORKSPACE_TASKMANAGER_LIBRARY}
    solidcontrol
    kworkspace)

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
  Serializator.cpp
)

kde4_add_ui_files(
  plasma_applet_lancelot_part_SRCS
  parts/LancelotPartConfigBase.ui
)

kde4_add_plugin(plasma_applet_lancelot_part ${plasma_applet_lancelot_part_SRCS})

target_link_libraries(
    plasma_applet_lancelot_part
    ${KDE4_PLASMA_LIBS}
    ${LANCELOT_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4WORKSPACE_TASKMANAGER_LIBRARY}
    solidcontrol
    kworkspace)

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
  Serializator.cpp
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
   ${LANCELOT_LIBS}
   ${KDE4_KIO_LIBS}
   ${KDE4_SOLID_LIBS}
   ${KDE4WORKSPACE_TASKMANAGER_LIBRARY}
   solidcontrol
   kworkspace
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
