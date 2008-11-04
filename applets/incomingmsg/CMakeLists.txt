# Project Needs a name ofcourse
project(plasma-incomingmsg)

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
set(incomingmsg_SRCS incomingmsg.cpp)

# Now make sure all files get to the right place
kde4_add_ui_files(incomingmsg_SRCS widget.ui)
kde4_add_plugin(plasma_applet_incomingmsg ${incomingmsg_SRCS})

target_link_libraries(plasma_applet_incomingmsg
                      ${QT_QTNETWORK_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_incomingmsg
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-incomingmsg.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
