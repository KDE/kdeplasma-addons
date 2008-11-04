set(krunner_browserhistory_SRCS
    browserhistory.cpp
)

kde4_add_plugin(krunner_browserhistory ${krunner_browserhistory_SRCS})
target_link_libraries(krunner_browserhistory ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krunner_browserhistory DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES browserhistory.desktop DESTINATION ${SERVICES_INSTALL_DIR})

