project(LancelotMenu)

IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

# Lancelot Menu

set(
  plasma_lancelot_SRCS
  main.cpp

  Global.cpp
  Widget.cpp
  WidgetPositioner.cpp

  LancelotApplication.cpp
  LancelotWindow.cpp

  CardLayout.cpp

  Panel.cpp
  BaseActionWidget.cpp

  ExtenderButton.cpp
  ToggleExtenderButton.cpp

  ActionListView.cpp
  ActionListViewModels.cpp

  PassagewayView.cpp
  PassagewayViewModels.cpp

# Display
  QtDisplay.cpp

# Models
  models/BaseModel.cpp
  models/FolderModel.cpp

# Computer
  models/Devices.cpp
  models/Places.cpp
  models/SystemServices.cpp

# Documents
  models/RecentDocuments.cpp
  models/OpenDocuments.cpp
  models/NewDocuments.cpp

# Runner (Search)
  models/Runner.cpp

)

QT4_ADD_DBUS_ADAPTOR(plasma_lancelot_SRCS org.kde.lancelot.xml
                     LancelotApplication.h LancelotApplication)

set(screensaver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(plasma_lancelot_SRCS ${screensaver_xml} screensaver_interface)

set(screensaver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.krunner.Interface.xml)
QT4_ADD_DBUS_INTERFACE(plasma_lancelot_SRCS ${screensaver_xml} krunner_interface)

set(ksmserver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(plasma_lancelot_SRCS ${ksmserver_xml} ksmserver_interface)

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

## add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

#include_directories(
#  ${CMAKE_SOURCE_DIR}
#  ${CMAKE_BINARY_DIR}
#  ${KDE4_INCLUDES}
#)

set(plasma_applet_lancelot_part_SRCS ${plasma_lancelot_SRCS} parts/lancelot.cpp)

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
