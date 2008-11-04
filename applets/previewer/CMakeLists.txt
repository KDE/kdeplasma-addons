# Project Needs a name ofcourse
project(previewer)
 
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
set(previewer_SRCS plasma-previewer.cpp
                   previewitemmodel.cpp
                   previewdialog.cpp
                   previewwidget.cpp
                   )

QT4_ADD_DBUS_ADAPTOR(Previewer_SRCS org.kde.Previewer.xml
                     plasma-previewer.h Previewer)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_previewer ${previewer_SRCS} ${Previewer_SRCS})


target_link_libraries(plasma_applet_previewer
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KIO_LIBS} ${QT_QTDBUS_LIBRARY})
 

install(TARGETS plasma_applet_previewer
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-previewer.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES preview.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

install(FILES
        previewer-16.svgz
        previewer-22.svgz
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

kde4_install_icons(${ICON_INSTALL_DIR})

