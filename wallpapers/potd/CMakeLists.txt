project(plasma-wallpaper-potd)

set(potd_SRCS
    potd.cpp
)
kde4_add_ui_files(potd_SRCS config.ui)

kde4_add_plugin(plasma_wallpaper_potd ${potd_SRCS})
target_link_libraries(plasma_wallpaper_potd ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_wallpaper_potd DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-potd.desktop DESTINATION ${SERVICES_INSTALL_DIR})
