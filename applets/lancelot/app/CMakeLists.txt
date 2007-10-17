project(LancelotMenu)

IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

add_subdirectory(theme)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(
  plasma_lancelot_SRCS
  main.cpp
  LancelotApplication.cpp
  LancelotWindow.cpp
  CardLayout.cpp
  ExtenderButton.cpp
  BaseActionWidget.cpp
  ActionListView.cpp
  Panel.cpp
  Global.cpp
  Widget.cpp
)

QT4_ADD_DBUS_ADAPTOR(plasma_lancelot_SRCS org.kde.lancelot.xml
                     LancelotApplication.h LancelotApplication)

kde4_add_executable(lancelot ${plasma_lancelot_SRCS})

target_link_libraries(lancelot ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS})

## DBUS service

configure_file(org.kde.lancelot.service.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lancelot.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lancelot.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services )

########### install applicaiton ###############
install(TARGETS lancelot DESTINATION ${BIN_INSTALL_DIR} )
