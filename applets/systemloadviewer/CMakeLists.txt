project(systemloadviewer)

set(systemloadviewer_SRCS
    systemloadviewer.cpp)

#find_package(KDE4 REQUIRED)

kde4_add_ui_files(systemloadviewer_SRCS generalconfig.ui coloursconfig.ui)
kde4_add_plugin(plasma-applet_systemloadviewer ${systemloadviewer_SRCS})
target_link_libraries(plasma-applet_systemloadviewer
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma-applet_systemloadviewer
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-systemloadviewer.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
