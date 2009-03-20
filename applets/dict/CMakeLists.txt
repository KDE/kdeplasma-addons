project(plasma-dict)

INCLUDE_DIRECTORIES(
	${QT_QTWEBKIT_INCLUDE_DIR})

set(dict_SRCS
    dict.cpp)

kde4_add_plugin(plasma_applet_dict ${dict_SRCS})
target_link_libraries(plasma_applet_dict ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS}
)

install(TARGETS plasma_applet_dict DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dict-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})
