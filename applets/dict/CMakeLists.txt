project(plasma-dict)

INCLUDE_DIRECTORIES(
	${QT_QTWEBKIT_INCLUDE_DIR})

set(dict_SRCS
    dict.cpp)


kde4_add_ui_files(dict_SRCS config.ui )

kde4_add_plugin(plasma_applet_dict ${dict_SRCS})
target_link_libraries(plasma_applet_dict ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS}
	QtWebKit)

install(TARGETS plasma_applet_dict DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dict-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
