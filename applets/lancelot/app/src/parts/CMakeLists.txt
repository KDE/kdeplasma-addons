project(plasma-applet-lancelot-part)

find_package(KDE4 REQUIRED)
find_package(Plasma REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE4_INCLUDES}
)

set(lancelot_SRCS lancelot.cpp)

kde4_add_plugin(plasma_applet_lancelot_part ${lancelot_SRCS})

target_link_libraries(plasma_applet_lancelot_part ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(
  TARGETS plasma_applet_lancelot_part
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
 FILES plasma-applet-lancelot-part.desktop
 DESTINATION ${SERVICES_INSTALL_DIR}
)
