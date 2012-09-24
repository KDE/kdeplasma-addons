cmake_minimum_required(VERSION 2.6)
project(plasma-wallpaper-virus)

set(virus_SRCS
    virus.cpp
    backgrounddelegate.cpp
    backgroundlistmodel.cpp
    alife.cpp
    itemsview.cpp
)
kde4_add_ui_files(virus_SRCS virusconfig.ui)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(image_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_plugin(plasma_wallpaper_virus ${virus_SRCS})
target_link_libraries(plasma_wallpaper_virus ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KNEWSTUFF3_LIBS} ${KDE4_THREADWEAVER_LIBRARY})

install(TARGETS plasma_wallpaper_virus DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-virus.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES virus_wallpaper.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
