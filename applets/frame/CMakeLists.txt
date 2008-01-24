project(plasma-frame)

set(frame_SRCS
    frame.cpp)


kde4_add_ui_files(frame_SRCS config.ui )

kde4_add_plugin(plasma_applet_frame ${frame_SRCS})
target_link_libraries(plasma_applet_frame ${PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS})

install(TARGETS plasma_applet_frame DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-frame-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES picture-frame-default.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
