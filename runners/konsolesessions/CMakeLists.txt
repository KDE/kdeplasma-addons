set(krunner_konsolesessions_SRCS
    konsolesessions.cpp
)

kde4_add_plugin(krunner_konsolesessions ${krunner_konsolesessions_SRCS})
target_link_libraries(krunner_konsolesessions ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krunner_konsolesessions DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES konsolesessions.desktop DESTINATION ${SERVICES_INSTALL_DIR})

