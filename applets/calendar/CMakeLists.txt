project(plasma-applet-calendar)

set(calendar_SRCS
    calendar.cpp)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
find_package(Plasma REQUIRED)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

kde4_add_plugin(plasma_applet_calendar ${calendar_SRCS})
target_link_libraries(plasma_applet_calendar ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} )

install(TARGETS plasma_applet_calendar DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR})
