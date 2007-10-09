project(plasma-kolourpicker)

set(kolourpicker_SRCS
    kolourpicker.cpp
)

kde4_add_plugin(plasma_applet_kolourpicker ${kolourpicker_SRCS})
target_link_libraries(plasma_applet_kolourpicker ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_kolourpicker DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-kolourpicker-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
