set(spellcheck_SRCS SpellCheck.cpp)

kde4_add_plugin(plasma_applet_spellcheck ${spellcheck_SRCS})
target_link_libraries(plasma_applet_spellcheck ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KCMUTILS_LIBS})

install(TARGETS plasma_applet_spellcheck DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-spellcheck.desktop DESTINATION ${SERVICES_INSTALL_DIR})
