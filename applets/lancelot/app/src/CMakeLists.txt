project(LancelotMenu)

include(PlasmaUIMacros)

IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

# Lancelot UI Widgets files
set(plasma_lancelot_widgets_SRCS
  Global.cpp
  Widget.cpp
  WidgetPositioner.cpp

  CardLayout.cpp
  ColumnLayout.cpp

  Panel.cpp
  BaseActionWidget.cpp

  ExtenderButton.cpp
  ToggleExtenderButton.cpp

  ActionListView.cpp
  ActionListViewModels.cpp

  PassagewayView.cpp
  PassagewayViewModels.cpp
)

# Lancelot data models files
set(plasma_lancelot_models_SRCS
  models/BaseModel.cpp
  models/FolderModel.cpp

  models/FavoriteApplications.cpp
  models/Applications.cpp

  models/Devices.cpp
  models/Places.cpp
  models/SystemServices.cpp

  models/RecentDocuments.cpp
  models/OpenDocuments.cpp
  models/NewDocuments.cpp

  models/Runner.cpp
)

# Lancelot application files
set(plasma_lancelot_application_SRCS
  LancelotApplication.cpp
  LancelotWindow.cpp
  QtDisplay.cpp
)

KDE4_ADD_PLASMA_UI_FILES(plasma_lancelot_application_SRCS
  "ui/LancelotWindowBase.xml"
)

# Lancelot client files
set(plasma_lancelot_client_SRCS
)

# Lancelot D-Bus connections
#set(plasma_lancelot_dbus_SRCS)

set(screensaver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(plasma_lancelot_application_SRCS ${screensaver_xml} screensaver_interface)

set(screensaver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.krunner.Interface.xml)
QT4_ADD_DBUS_INTERFACE(plasma_lancelot_application_SRCS ${screensaver_xml} krunner_interface)

set(ksmserver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(plasma_lancelot_application_SRCS ${ksmserver_xml} ksmserver_interface)

set(lancelot_xml org.kde.lancelot.xml)
QT4_ADD_DBUS_INTERFACE(plasma_lancelot_client_SRCS ${lancelot_xml} lancelot_interface)

QT4_ADD_DBUS_ADAPTOR(plasma_lancelot_application_SRCS org.kde.lancelot.xml
     LancelotApplication.h LancelotApplication)

# Lancelot Menu
set(
  plasma_lancelot_SRCS
  main.cpp

  ${plasma_lancelot_widgets_SRCS}
  ${plasma_lancelot_models_SRCS}
  ${plasma_lancelot_application_SRCS}
)


kde4_add_executable(lancelot ${plasma_lancelot_SRCS})

target_link_libraries(lancelot ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} taskmanager)

## DBUS service

configure_file(org.kde.lancelot.service.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lancelot.service)

# To regenerate .xml file:
# qdbuscpp2xml -M -s LancelotApplication.h -o org.kde.lancelot.xml.new
# And make the needed changes
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lancelot.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR}) # ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services )

########### install applicaiton ###############
install(TARGETS lancelot DESTINATION ${BIN_INSTALL_DIR} )


# Lancelot Part Applet

set(plasma_applet_lancelot_part_SRCS
  ${plasma_lancelot_widgets_SRCS}
  ${plasma_lancelot_models_SRCS}
  ${plasma_lancelot_application_SRCS}
  parts/LancelotPart.cpp
)

kde4_add_plugin(plasma_applet_lancelot_part ${plasma_applet_lancelot_part_SRCS})

target_link_libraries(plasma_applet_lancelot_part ${PLASMA_LIBS} ${KDE4_KIO_LIBS} taskmanager)

install(
  TARGETS plasma_applet_lancelot_part
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
  FILES parts/plasma-applet-lancelot-part.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)

# Lancelot Main Applet

set(plasma_applet_lancelot_SRCS
  ${plasma_lancelot_widgets_SRCS}
  ${plasma_lancelot_client_SRCS}
  applet/LancelotApplet.cpp
  applet/LancelotAppletConfig.cpp
)

kde4_add_ui_files(
  plasma_applet_lancelot_SRCS
  applet/LancelotAppletConfigBase.ui
)


kde4_add_plugin(plasma_applet_lancelot ${plasma_applet_lancelot_SRCS})

target_link_libraries(plasma_applet_lancelot ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(
  TARGETS plasma_applet_lancelot
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
  FILES applet/plasma-applet-lancelot.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)
