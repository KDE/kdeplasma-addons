
set(krunner_placesrunner_SRCS
    placesrunner.cpp
)

kde4_add_plugin(krunner_placesrunner ${krunner_placesrunner_SRCS})
target_link_libraries(
    krunner_placesrunner
    ${KDE4_KFILE_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    )

install(TARGETS krunner_placesrunner DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-places.desktop DESTINATION ${SERVICES_INSTALL_DIR})

