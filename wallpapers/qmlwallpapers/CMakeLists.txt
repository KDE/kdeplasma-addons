project(plasma-wallpaper-qml)

kde4_add_ui_files(qmlwp_SRCS viewconfig.ui)

kde4_add_plugin(plasma_wallpaper_qml wallpapersmodel.cpp wallpapersqml.cpp backgrounddelegate.cpp ${qmlwp_SRCS})
target_link_libraries(plasma_wallpaper_qml
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} kdeclarative
)

install(TARGETS plasma_wallpaper_qml DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-qml.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(wallpapers)
