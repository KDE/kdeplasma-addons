
# Project needs a name of course
project(blackboard)

# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)


add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

# We add our source code here
set(blackboard_SRCS blackboard.cpp blackboardwidget.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_blackboard ${blackboard_SRCS})
target_link_libraries(plasma_applet_blackboard
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_blackboard
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-blackboard.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
